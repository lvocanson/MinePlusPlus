#include "BoardRenderer.h"
#include "App.h"
#include "Resources.h"
#include "Utils/SFMLExtensions.h"


BoardRenderer::BoardRenderer()
	: cheat_()
{
}

EventConsumed BoardRenderer::handleEvent(const sf::Event& event)
{
	auto* pressed = event.getIf<sf::Event::KeyPressed>();
	if (pressed && pressed->code == sf::Keyboard::Key::C)
	{
		cheat_ = !cheat_;
		return EventConsumed::Yes;
	}
	return EventConsumed::No;
}

void BoardRenderer::render(sf::RenderTarget& target) const
{
	auto& game = App::instance().game;
	bool reveal = cheat_ || game.isGameOver();
	auto& board = game.getBoard();
	auto& pressedCell = game.getPressedCell();
	std::size_t pressedCellIndex = board.areCoordinatesValid(pressedCell)
		? board.toIndex(pressedCell) : -1;

	for (std::size_t index = 0; index < board.getCells().size(); ++index)
	{
		const sf::Texture* texture = nullptr;
		auto& cell = board.getCellAt(index);

		if (cell.flagged)
		{
			texture = (reveal && !cell.mined)
				? &Resources::Textures::openedCellNoMine
				: &Resources::Textures::unopenedFlaggedCell;
		}

		else if (!cell.opened)
		{
			texture = (reveal && cell.mined)
				? &Resources::Textures::openedCellMine
				: (pressedCellIndex == index)
				? &Resources::Textures::unopenedSelectedCell
				: &Resources::Textures::unopenedCell;
		}

		else if (cell.mined)
		{
			texture = &Resources::Textures::openedCellClickedMine;
		}

		else
		{
			constexpr const sf::Texture* openedCellTextures[]
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
			texture = openedCellTextures[cell.adjacentMines];
		}

		assert(texture);

		auto [x, y] = board.toCoordinates(index);
		const sf::FloatRect rect{{float(x), float(y)}, {1.f, 1.f}};
		SFMLExtensions::drawTexture(target, *texture, rect);
	}
}
