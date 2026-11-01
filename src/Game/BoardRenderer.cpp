#include "BoardRenderer.h"
#include "Board.h"
#include "Core/View.h"
#include "Game/Resources.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <limits>

namespace
{

using Resources::Textures::Tile;

constexpr int FRACTION_BITS = 28;
constexpr std::uint64_t FRACTION_MASK = (std::uint64_t(1) << FRACTION_BITS) - 1;
constexpr double FIXED_ONE = double(std::int64_t(1) << FRACTION_BITS);
constexpr double MAX_ROW_START = double(std::int64_t(1) << 34);

std::int64_t toFixed(double value)
{
	return std::llround(value * FIXED_ONE);
}

constexpr std::uint8_t toByte(Tile tile)
{
	return static_cast<std::uint8_t>(tile);
}

Tile tileAt(const Board& board, const BoardRenderer::State& state, std::size_t index)
{
	using Reveal = BoardRenderer::Reveal;
	const Cell& cell = board.getCellAt(index);

	// Overrides the flag: a revealed running bomb always shows its own skin
	if (cell.mined && (state.reveal != Reveal::None || cell.opened)
	    && std::ranges::find(state.runningMineIndexes, index) != state.runningMineIndexes.end())
	{
		return Tile::OpenedRunningMine;
	}

	if (cell.flagged)
	{
		// Wrong flags are only called out on a loss, a win keeps them as is
		return (state.reveal == Reveal::Lost && !cell.mined)
		       ? Tile::OpenedNoMine
		       : Tile::UnopenedFlagged;
	}

	if (!cell.opened)
	{
		return (state.reveal != Reveal::None && cell.mined)
		       ? Tile::OpenedMine
		       : (state.pressedCellIndex == index)
		         ? Tile::UnopenedSelected
		         : Tile::Unopened;
	}

	if (cell.mined)
	{
		return Tile::OpenedClickedMine;
	}

	// Opened0 through Opened8 are consecutive
	assert(cell.adjacentMines < 9);
	return Tile(toByte(Tile::Opened0) + cell.adjacentMines);
}

} // namespace

// TODO: nearest sampling without mipmaps, detailed cells alias when zoomed out down to minDetailedZoom
// TODO: every pixel is tested against the board, clipping each row to the board would skip the empty space
void BoardRenderer::render(Canvas canvas, const View& view, const Board& board, const State& state)
{
	using Resources::Textures::atlasColumns;
	using Resources::Textures::cellsAtlas;
	const float minDetailedZoom = float(cellsAtlas.width / atlasColumns) / 4.f;
	const assets::IndexedImage& atlas = view.zoom < minDetailedZoom ? Resources::Textures::flatCellsAtlas : cellsAtlas;
	const int cellSize = atlas.width / atlasColumns;

	Vec2s boardSize = board.getSize();
	std::uint64_t limitX = std::uint64_t(boardSize.x) << FRACTION_BITS;
	std::uint64_t limitY = std::uint64_t(boardSize.y) << FRACTION_BITS;
	Transform toWorld = view.toWorld();
	std::int64_t stepX = toFixed(toWorld.a);
	std::int64_t stepY = toFixed(toWorld.d);

	std::size_t cachedIndex = std::numeric_limits<std::size_t>::max();
	const std::uint8_t* tileIndices = nullptr;

	for (int y = 0; y < canvas.size.y; ++y)
	{
		std::uint32_t* row = canvas.row(y);
		Vec2d rowStart = toWorld.apply({0.5, double(y) + 0.5});
		if (!(std::abs(rowStart.x) < MAX_ROW_START && std::abs(rowStart.y) < MAX_ROW_START))
			continue;

		std::int64_t worldX = toFixed(rowStart.x);
		std::int64_t worldY = toFixed(rowStart.y);
		for (int x = 0; x < canvas.size.x; ++x, worldX += stepX, worldY += stepY)
		{
			auto fixedX = std::uint64_t(worldX), fixedY = std::uint64_t(worldY);
			if (fixedX >= limitX || fixedY >= limitY)
				continue;

			std::size_t index = std::size_t(fixedX >> FRACTION_BITS) + std::size_t(fixedY >> FRACTION_BITS) * boardSize.x;
			if (index != cachedIndex)
			{
				cachedIndex = index;
				int tile = toByte(tileAt(board, state, index));
				Vec2i origin = Vec2i{tile % atlasColumns, tile / atlasColumns} * cellSize;
				tileIndices = atlas.indices.data() + std::size_t(origin.y) * std::size_t(atlas.width) + std::size_t(origin.x);
			}

			int u = int(((fixedX & FRACTION_MASK) * std::uint64_t(cellSize)) >> FRACTION_BITS);
			int v = int(((fixedY & FRACTION_MASK) * std::uint64_t(cellSize)) >> FRACTION_BITS);
			row[x] = atlas.palette[tileIndices[v * atlas.width + u]];
		}
	}
}
