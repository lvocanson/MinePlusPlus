#pragma once
#include <SFML/Audio/SoundBuffer.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Shader.hpp>

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
constexpr sf::FloatRect openedCell0{cellSize.componentWiseMul({0, 0}), cellSize};
constexpr sf::FloatRect openedCell1{cellSize.componentWiseMul({1, 0}), cellSize};
constexpr sf::FloatRect openedCell2{cellSize.componentWiseMul({2, 0}), cellSize};
constexpr sf::FloatRect openedCell3{cellSize.componentWiseMul({3, 0}), cellSize};
constexpr sf::FloatRect openedCell4{cellSize.componentWiseMul({0, 1}), cellSize};
constexpr sf::FloatRect openedCell5{cellSize.componentWiseMul({1, 1}), cellSize};
constexpr sf::FloatRect openedCell6{cellSize.componentWiseMul({2, 1}), cellSize};
constexpr sf::FloatRect openedCell7{cellSize.componentWiseMul({3, 1}), cellSize};
constexpr sf::FloatRect openedCell8{cellSize.componentWiseMul({0, 2}), cellSize};
constexpr sf::FloatRect unopenedCell{cellSize.componentWiseMul({1, 2}), cellSize};
constexpr sf::FloatRect unopenedSelectedCell{cellSize.componentWiseMul({2, 2}), cellSize};
constexpr sf::FloatRect unopenedFlaggedCell{cellSize.componentWiseMul({3, 2}), cellSize};
constexpr sf::FloatRect openedCellRunningMine{cellSize.componentWiseMul({0, 3}), cellSize};
constexpr sf::FloatRect openedCellMine{cellSize.componentWiseMul({1, 3}), cellSize};
constexpr sf::FloatRect openedCellClickedMine{cellSize.componentWiseMul({2, 3}), cellSize};
constexpr sf::FloatRect openedCellNoMine{cellSize.componentWiseMul({3, 3}), cellSize};

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
 * Expands each group of 6 vertices into a textured quad. Every vertex of a
 * group carries the same cell position and the same atlas offset (in pixels);
 * the corner is derived from gl_VertexID.
 *
 * gl_VertexID requires GLSL 1.30. The gl_Vertex / gl_Color / gl_MultiTexCoord0
 * attributes are deprecated there but still fed by SFML's fixed-function vertex
 * arrays. The fragment stage is explicit: a #version 130 vertex shader cannot
 * reliably hand gl_TexCoord over to fixed-function fragment processing.
 */
inline sf::Shader cell()
{
	return {
		std::string_view(
			R"(#version 130

uniform vec2 atlasCellSize;
uniform vec2 atlasTexSize;

out vec2 vUV;
out vec4 vColor;

void main()
{
    int localID = gl_VertexID % 6;
    vec2 offset;
    if(localID == 0) offset = vec2(0,0);
    else if(localID == 1) offset = vec2(1,0);
    else if(localID == 2) offset = vec2(0,1);
    else if(localID == 3) offset = vec2(1,0);
    else if(localID == 4) offset = vec2(1,1);
    else offset = vec2(0,1);

    vec2 pos = gl_Vertex.xy + offset;
    gl_Position = gl_ModelViewProjectionMatrix * vec4(pos, 0.0, 1.0);

    // Half texel inset: the corners map to the centers of the outermost texels
    // instead of the cell edges, so the interpolated UV can never reach the
    // neighbouring atlas cell whatever the filtering or the view transform.
    vec2 uvPx = gl_MultiTexCoord0.xy + offset * (atlasCellSize - 1.0) + 0.5;
    vUV = uvPx / atlasTexSize;
    vColor = gl_Color;
})"),
		std::string_view(
			R"(#version 130

uniform sampler2D atlasTex;

in vec2 vUV;
in vec4 vColor;

out vec4 fragColor;

void main()
{
    fragColor = texture(atlasTex, vUV) * vColor;
})")
	};
};

} // namespace Shaders

} // namespace Resources
