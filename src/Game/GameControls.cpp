#include "GameControls.h"
#include "Core/App.h"
#include "Game/Resources.h"
#include <SFML/Graphics/View.hpp>

namespace
{

// World coordinates left of or above the board are not representable as a Vec2s.
std::optional<Vec2s> toCellCoordinates(sf::Vector2f position)
{
	if (position.x < 0.f || position.y < 0.f)
		return std::nullopt;
	return Vec2s{std::size_t(position.x), std::size_t(position.y)};
}

} // namespace

void GameControls::operator()(const WorldEvent::Pressed& event)
{
	switch (event.button)
	{
	case sf::Mouse::Button::Left:
	case sf::Mouse::Button::Right:
	{
		auto& board = event.app.getGame().getBoard();
		auto coo = toCellCoordinates(event.position);
		if (coo && board.areCoordinatesValid(*coo))
		{
			event.app.getGame().setPressedCell(*coo);
		}
	}
	break;
	}
}

void GameControls::operator()(const WorldEvent::Released& event)
{
	switch (event.button)
	{
	case sf::Mouse::Button::Left:
	case sf::Mouse::Button::Right:
	{
		auto pressedCell = event.app.getGame().getPressedCell();
		event.app.getGame().setPressedCell(std::nullopt);

		auto& board = event.app.getGame().getBoard();
		auto coo = toCellCoordinates(event.position);

		// Only continue if we release the mouse on the previously pressed cell
		if (!coo || pressedCell != *coo)
			return;

		if (event.button == sf::Mouse::Button::Right)
		{
			auto flags = board.getFlagCount();
			event.app.getGame().flag(*coo);
			if (flags != board.getFlagCount())
				event.app.getAudio().play(Resources::Sounds::click2);
			return;
		}

		auto opened = board.getOpenCount();
		event.app.getGame().open(*coo);
		if (opened != board.getOpenCount())
		{
			auto& game = event.app.getGame();
			if (game.isGameOver())
			{
				event.app.getAudio().play(
					game.isLost()
					? Resources::Sounds::explosion
					: Resources::Sounds::victory);
			}
			else
			{
				event.app.getAudio().play(Resources::Sounds::click1);
			}
		}
	}
	break;
	}
}
