#include "BoardRenderer.h"
#include "Board.h"
#include "Game/Resources.h"
#include <algorithm>
#include <cassert>

namespace
{

using Resources::Textures::Tile;

// The state texture is RGBA8, the smallest format SFML exposes, and holds the
// board as one flat byte array: four consecutive cells share a texel, one Tile
// per channel. Laying it out flat rather than one texture row per board row
// avoids padding every row, and keeps the height far below GL_MAX_TEXTURE_SIZE
// for boards that are very tall but narrow.
constexpr std::size_t CELLS_PER_TEXEL = 4;

// Power of two, so the shader can address a cell with shifts and masks only.
constexpr std::size_t STATE_TEX_WIDTH_LOG2 = 10;
constexpr std::size_t STATE_TEX_WIDTH = std::size_t(1) << STATE_TEX_WIDTH_LOG2;
constexpr std::size_t CELLS_PER_TEX_ROW = STATE_TEX_WIDTH * CELLS_PER_TEXEL;

constexpr std::uint8_t toByte(Tile tile)
{
	return static_cast<std::uint8_t>(tile);
}

Tile tileAt(const Board& board, const BoardRenderer::State& state, std::size_t index)
{
	using Reveal = BoardRenderer::Reveal;
	const Cell& cell = board.getCellAt(index);

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

BoardRenderer::BoardRenderer()
	: scratch_{}
	, shader_(Resources::Shaders::cell())
	, dirty_(true)
{
	shader_.setUniform("atlasTex", sf::Shader::CurrentTexture);
	shader_.setUniform("atlasCellSize", Resources::Textures::cellSize);
	shader_.setUniform("atlasTexSize", sf::Vector2f(Resources::Textures::cellsAtlas.getSize()));
	shader_.setUniform("stateTexWidthLog2", int(STATE_TEX_WIDTH_LOG2));
}

void BoardRenderer::resize(const Board& board)
{
	Vec2s size = board.getSize();

	boardQuad_.setPrimitiveType(sf::PrimitiveType::TriangleStrip);
	boardQuad_.resize(4);
	boardQuad_[0].position = {0.f, 0.f};
	boardQuad_[1].position = {float(size.x), 0.f};
	boardQuad_[2].position = {0.f, float(size.y)};
	boardQuad_[3].position = {float(size.x), float(size.y)};

	// Fails past GL_MAX_TEXTURE_SIZE rows, which is 16384 cells of height on most
	// drivers: a full 67 million cells at this texture width.
	std::size_t cellCount = size.x * size.y;
	std::size_t texRows = (cellCount + CELLS_PER_TEX_ROW - 1) / CELLS_PER_TEX_ROW;
	[[maybe_unused]] bool resized = stateTexture_.resize({unsigned(STATE_TEX_WIDTH), unsigned(texRows)});
	assert(resized);

	// Rebinds the state texture: resizing it gives the shader a new GL object.
	shader_.setUniform("stateTex", stateTexture_);
	shader_.setUniform("boardSize", sf::Vector2f(float(size.x), float(size.y)));

	// The freshly allocated texture holds garbage until the first upload.
	dirty_ = true;
}

void BoardRenderer::update(const Board& board, const State& state)
{
	if (!dirty_) return;
	dirty_ = false;

	Vec2s size = board.getSize();
	for (std::size_t first = 0, cellCount = size.x * size.y; first < cellCount; first += scratch_.size())
	{
		std::size_t last = std::min(first + scratch_.size(), cellCount);

		for (std::size_t index = first; index < last; ++index)
			scratch_[index - first] = toByte(tileAt(board, state, index));

		// Running bombs are patched in place rather than in a second pass over the
		// board: there are only a handful of them, so skipping the ones outside the
		// block is cheaper than sorting the list.
		for (std::size_t index : state.runningMineIndexes)
		{
			if (index < first || index >= last)
				continue;

			const Cell& cell = board.getCellAt(index);
			assert(cell.mined);

			// Overrides the flag: a revealed running bomb always shows its own skin
			if (state.reveal != Reveal::None || cell.opened)
				scratch_[index - first] = toByte(Tile::OpenedRunningMine);
		}

		flushScratch(first, last - first);
	}
}

void BoardRenderer::flushScratch(std::size_t first, std::size_t count)
{
	// A load goes up as a single run of texels inside one texture row, so a row
	// must be a whole number of loads for a run to never straddle two rows.
	static_assert(CELLS_PER_TEX_ROW % std::tuple_size_v<decltype(scratch_)> == 0);
	assert(first % scratch_.size() == 0 && count <= scratch_.size());

	std::size_t firstTexel = first / CELLS_PER_TEXEL;
	std::size_t texels = (count + CELLS_PER_TEXEL - 1) / CELLS_PER_TEXEL;

	// A board rarely ends on a texel boundary, and only whole texels can be
	// uploaded. Those trailing cells do not exist, but leaving the previous load
	// behind would put stale Tiles in the texture.
	using diff = decltype(scratch_)::difference_type;
	std::fill(scratch_.begin() + diff(count), scratch_.begin() + diff(texels * CELLS_PER_TEXEL), toByte(Tile::Unopened));

	stateTexture_.update(
		scratch_.data(),
		{unsigned(texels), 1},
		{unsigned(firstTexel % STATE_TEX_WIDTH), unsigned(firstTexel / STATE_TEX_WIDTH)});
}

void BoardRenderer::render(sf::RenderTarget& target) const
{
	sf::RenderStates states;
	states.texture = &Resources::Textures::cellsAtlas;
	states.shader = &shader_;
	target.draw(boardQuad_, states);
}
