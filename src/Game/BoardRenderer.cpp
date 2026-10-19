#include "BoardRenderer.h"
#include "Board.h"
#include "Core/View.h"
#include "Game/Resources.h"
#include <algorithm>
#include <cassert>
#include <limits>

namespace
{

using Resources::Textures::Tile;

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

// TODO: nearest sampling without mipmaps, zoomed out boards alias
// TODO: every pixel is tested against the board, clipping each row to the board would skip the empty space
void BoardRenderer::render(Canvas canvas, const View& view, const Board& board, const State& state)
{
	using Resources::Textures::cellsAtlas;
	constexpr int CELL_SIZE = Resources::Textures::cellSize;
	const int atlasColumns = cellsAtlas.size.x / CELL_SIZE;

	Vec2s boardSize = board.getSize();
	Vec2f boardLimit(boardSize);
	Transform toWorld = view.toWorld();

	std::size_t cachedIndex = std::numeric_limits<std::size_t>::max();
	const std::uint32_t* tilePixels = nullptr;

	for (int y = 0; y < canvas.size.y; ++y)
	{
		std::uint32_t* row = canvas.row(y);
		Vec2f rowStart = toWorld.apply({0.f, float(y) + 0.5f});

		for (int x = 0; x < canvas.size.x; ++x)
		{
			float fx = float(x) + 0.5f;
			Vec2f world{rowStart.x + toWorld.a * fx, rowStart.y + toWorld.d * fx};
			if (!(world.x >= 0.f && world.y >= 0.f && world.x < boardLimit.x && world.y < boardLimit.y))
				continue;

			Vec2s cell(world);
			std::size_t index = cell.x + cell.y * boardSize.x;
			if (index != cachedIndex)
			{
				cachedIndex = index;
				int tile = toByte(tileAt(board, state, index));
				Vec2i origin = Vec2i{tile % atlasColumns, tile / atlasColumns} * CELL_SIZE;
				tilePixels = cellsAtlas.pixels.data() + std::size_t(origin.y) * std::size_t(cellsAtlas.size.x) + std::size_t(origin.x);
			}

			int u = std::min(int((world.x - float(cell.x)) * CELL_SIZE), CELL_SIZE - 1);
			int v = std::min(int((world.y - float(cell.y)) * CELL_SIZE), CELL_SIZE - 1);
			row[x] = tilePixels[v * cellsAtlas.size.x + u];
		}
	}
}
