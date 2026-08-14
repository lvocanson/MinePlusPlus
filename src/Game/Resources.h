#pragma once
#include <SFML/Audio/SoundBuffer.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Shader.hpp>
#include <cstdint>

/*
 * Resources are automatically constructed before main().
 * Ensures all required game resources (fonts, textures, etc.) are loaded.
 * If any mandatory resource cannot be found, the application will fail to start.
 */
namespace Resources
{

inline const std::filesystem::path RESOURCES_DIR = "res";

namespace Fonts
{

constexpr auto ROBOTO_FILE = "roboto-v48-latin-regular.ttf";

inline const sf::Font roboto{RESOURCES_DIR / ROBOTO_FILE};

} // namespace Fonts

namespace Textures
{

constexpr auto CELLS_ATLAS_FILE = "cells-atlas.png";

inline const sf::Texture cellsAtlas{RESOURCES_DIR / CELLS_ATLAS_FILE};

constexpr sf::Vector2f cellSize = {64, 64};

/*
 * Skin of a cell, as its row-major index inside the atlas. This is the only
 * thing the renderer stores per cell, so the order below must match the atlas
 * image: the shader turns the index back into a pixel offset by dividing by
 * the number of columns. Keep the list within 16 entries, otherwise it no
 * longer fits in the nibble the state texture reserves for it.
 */
enum class Tile : std::uint8_t
{
	Opened0, Opened1, Opened2, Opened3,
	Opened4, Opened5, Opened6, Opened7,
	Opened8, Unopened, UnopenedSelected, UnopenedFlagged,
	OpenedRunningMine, OpenedMine, OpenedClickedMine, OpenedNoMine,
};

} // namespace Textures

namespace Sounds
{

constexpr auto CLICK_1_FILE = "click-1.ogg";
constexpr auto CLICK_2_FILE = "click-2.ogg";
constexpr auto EXPLOSION_FILE = "explosion.ogg";
constexpr auto VICTORY_FILE = "victory.ogg";

inline const sf::SoundBuffer click1{RESOURCES_DIR / CLICK_1_FILE};
inline const sf::SoundBuffer click2{RESOURCES_DIR / CLICK_2_FILE};
inline const std::filesystem::path explosion{RESOURCES_DIR / EXPLOSION_FILE};
inline const std::filesystem::path victory{RESOURCES_DIR / VICTORY_FILE};

} // namespace Sounds

namespace Shaders
{

/*
 * Draws the whole board as a single quad spanning [0, boardSize] in cell units.
 * The fragment stage figures out which cell it lands in, reads that cell's Tile
 * from the state texture, and samples the matching atlas cell.
 *
 * The state texture is an RGBA8 image holding the board as one flat byte array,
 * four cells per texel and one Tile per channel, so the board costs one byte per
 * cell on the GPU instead of a vertex group. Going flat rather than one texture
 * row per board row keeps the texture height low whatever the board shape.
 *
 * texelFetch / textureGrad / bit operators require GLSL 1.30. The gl_Vertex and
 * gl_ModelViewProjectionMatrix built-ins are deprecated there but still fed by
 * SFML's fixed-function vertex arrays. The fragment stage is explicit: a
 * #version 130 vertex shader cannot reliably hand gl_TexCoord over to
 * fixed-function fragment processing.
 */
inline sf::Shader cell()
{
	return {
		std::string_view(
			R"(#version 130

out vec2 vBoardPos;

void main()
{
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
    vBoardPos = gl_Vertex.xy;
})"),
		std::string_view(
			R"(#version 130

uniform sampler2D atlasTex;
uniform sampler2D stateTex;
uniform int stateTexWidthLog2;
uniform vec2 boardSize;
uniform vec2 atlasCellSize;
uniform vec2 atlasTexSize;

in vec2 vBoardPos;
out vec4 fragColor;

void main()
{
    // Which cell this fragment lands in, and where inside it. The clamps only
    // matter for fragments sitting exactly on the far edge of the quad.
    vec2 cellPos = clamp(floor(vBoardPos), vec2(0.0), boardSize - 1.0);
    vec2 local = clamp(vBoardPos - cellPos, 0.0, 1.0);

    // The state texture is a flat byte array, four cells per texel. Its width is
    // a power of two, so addressing a cell stays shifts and masks.
    int cellIndex = int(cellPos.x) + int(boardSize.x) * int(cellPos.y);
    int texelIndex = cellIndex >> 2;
    ivec2 stateTexel = ivec2(texelIndex & ((1 << stateTexWidthLog2) - 1),
                             texelIndex >> stateTexWidthLog2);
    vec4 packedTiles = texelFetch(stateTex, stateTexel, 0);
    int tile = int(packedTiles[cellIndex & 3] * 255.0 + 0.5);

    // Atlas cells are laid out row-major, so the Tile is a linear index into it.
    int columns = int(atlasTexSize.x / atlasCellSize.x);
    vec2 tileOrigin = vec2(tile % columns, tile / columns) * atlasCellSize;

    // Half texel inset: local 0 and 1 map to the centers of the outermost texels
    // instead of the cell edges, so the UV can never reach the neighbouring
    // atlas cell whatever the filtering or the view transform.
    vec2 uv = (tileOrigin + local * (atlasCellSize - 1.0) + 0.5) / atlasTexSize;

    // 'local' wraps back to 0 at every cell border, which would make the implicit
    // derivatives explode there. Derive them from the continuous board position.
    vec2 uvPerCell = (atlasCellSize - 1.0) / atlasTexSize;
    fragColor = textureGrad(atlasTex, uv, dFdx(vBoardPos) * uvPerCell, dFdy(vBoardPos) * uvPerCell);
})")
	};
};

} // namespace Shaders

} // namespace Resources
