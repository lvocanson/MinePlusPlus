#include "BoardRenderer.h"
#include "Board.h"
#include "Game/Resources.h"
#include <cassert>

namespace
{

// The cell shader expands each group of 6 vertices into a textured quad,
// so every vertex of a group carries the same cell position and atlas offset.
constexpr std::size_t VERTICES_PER_CELL = 6;

} // namespace

BoardRenderer::BoardRenderer()
	: shader_(Resources::Shaders::cell())
{
	shader_.setUniform("atlasTex", sf::Shader::CurrentTexture);
	shader_.setUniform("atlasCellSize", Resources::Textures::cellSize);
	shader_.setUniform("atlasTexSize", sf::Vector2f(Resources::Textures::cellsAtlas.getSize()));
}

void BoardRenderer::resize(const Board& board)
{
	Vec2s size = board.getSize();
	varray_.setPrimitiveType(sf::PrimitiveType::Triangles);
	varray_.resize(size.x * size.y * VERTICES_PER_CELL);

	for (std::size_t y = 0; y < size.y; ++y)
	{
		for (std::size_t x = 0; x < size.x; ++x)
		{
			std::size_t first = (x + size.x * y) * VERTICES_PER_CELL;
			for (std::size_t i = 0; i < VERTICES_PER_CELL; ++i)
				varray_[first + i].position = {float(x), float(y)};
		}
	}
}

void BoardRenderer::setCellTexCoords(std::size_t index, const sf::FloatRect& atlasRect)
{
	std::size_t first = index * VERTICES_PER_CELL;
	for (std::size_t i = 0; i < VERTICES_PER_CELL; ++i)
		varray_[first + i].texCoords = atlasRect.position;
}

void BoardRenderer::update(const Board& board, const State& state)
{
	for (std::size_t index = 0; index < board.getCells().size(); ++index)
	{
		const sf::FloatRect* atlasRect = nullptr;
		auto& cell = board.getCellAt(index);

		if (cell.flagged)
		{
			// Wrong flags are only called out on a loss, a win keeps them as is
			atlasRect = (state.reveal == Reveal::Lost && !cell.mined)
			            ? &Resources::Textures::openedCellNoMine
			            : &Resources::Textures::unopenedFlaggedCell;
		}

		else if (!cell.opened)
		{
			atlasRect = (state.reveal != Reveal::None && cell.mined)
			            ? &Resources::Textures::openedCellMine
			            : (state.pressedCellIndex == index)
			              ? &Resources::Textures::unopenedSelectedCell
			              : &Resources::Textures::unopenedCell;
		}

		else if (cell.mined)
		{
			atlasRect = &Resources::Textures::openedCellClickedMine;
		}

		else
		{
			constexpr const sf::FloatRect* openedCellTextures[]
			{
				&Resources::Textures::openedCell0,
				&Resources::Textures::openedCell1,
				&Resources::Textures::openedCell2,
				&Resources::Textures::openedCell3,
				&Resources::Textures::openedCell4,
				&Resources::Textures::openedCell5,
				&Resources::Textures::openedCell6,
				&Resources::Textures::openedCell7,
				&Resources::Textures::openedCell8,
			};

			assert(cell.adjacentMines < 9);
			atlasRect = openedCellTextures[cell.adjacentMines];
		}

		assert(atlasRect);
		setCellTexCoords(index, *atlasRect);
	}

	for (std::size_t index : state.runningMineIndexes)
	{
		auto& cell = board.getCellAt(index);
		assert(cell.mined);

		// Overrides the flag: a revealed running bomb always shows its own skin
		if (state.reveal != Reveal::None || cell.opened)
		{
			setCellTexCoords(index, Resources::Textures::openedCellRunningMine);
		}
	}
}

void BoardRenderer::render(sf::RenderTarget& target) const
{
	sf::RenderStates states;
	states.texture = &Resources::Textures::cellsAtlas;
	states.shader = &shader_;
	target.draw(varray_, states);
}
