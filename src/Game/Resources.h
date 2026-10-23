#pragma once
#include "Core/Audio.h"
#include "UI/Font.h"
#include <cells_atlas.h>
#include <cells_atlas_1px.h>
#include <click_1.h>
#include <click_2.h>
#include <explosion.h>
#include <victory.h>
#include <roboto_20.h>
#include <cstdint>

/*
 * Resources are automatically constructed before main().
 * Ensures all required game resources (fonts, textures, etc.) are loaded.
 * Fonts and textures are converted at build time, a missing one fails the build.
 */
namespace Resources
{

namespace Fonts
{

inline const Font roboto{assets::roboto_20()};

} // namespace Fonts

namespace Textures
{

inline const assets::IndexedImage& cellsAtlas = assets::cells_atlas();
inline const assets::IndexedImage& flatCellsAtlas = assets::cells_atlas_1px();

constexpr int atlasColumns = 4;

/*
 * Skin of a cell, as its row-major index inside the atlas. The order below must
 * match every atlas image: the renderer turns the index back into a pixel offset
 * by dividing by the number of columns.
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

inline const Audio::Sound click1{assets::click_1(), Audio::Track::Effect};
inline const Audio::Sound click2{assets::click_2(), Audio::Track::Effect};
inline const Audio::Sound explosion{assets::explosion(), Audio::Track::Music};
inline const Audio::Sound victory{assets::victory(), Audio::Track::Music};

} // namespace Sounds

} // namespace Resources
