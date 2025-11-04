#include "MinesweeperInput.h"
#include "Utils/Overloaded.h"
#include "App.h"

EventConsumed MinesweeperInput::handleEvent(const sf::Event& event)
{
	Overloaded visitor
	{
		[&](const sf::Event::MouseMovedRaw& event)
		{
			if (isMouseDraggingCamera_)
			{
				constexpr float sensitivity = 1.f;
				sf::View view = App::instance().window.getView();
				auto windowSize = sf::Vector2f(App::instance().window.getSize());
				// Assuming view and windowSize have the same aspect ratio
				float viewRatio = view.getSize().x / windowSize.x;
				auto offset = -(sensitivity * viewRatio) * sf::Vector2f(event.delta);
				view.move(offset.rotatedBy(view.getRotation()));
				App::instance().window.setView(view);
				return EventConsumed::Yes;
			}
			return EventConsumed::No;
		},

		[&](const sf::Event::MouseButtonPressed& event)
		{
			sf::Vector2f position = App::instance().window.mapPixelToCoords(event.position);
			switch (event.button)
			{
			case sf::Mouse::Button::Left:
			case sf::Mouse::Button::Right:
			{
				auto& board = App::instance().game.getBoard();
				Vec2s coo{std::size_t(position.x), std::size_t(position.y)};
				if (board.areCoordinatesValid(coo))
				{
					App::instance().game.setPressedCell(coo);
					return EventConsumed::Yes;
				}
			}
			break;

			case sf::Mouse::Button::Middle:
			{
				isMouseDraggingCamera_ = true;
				return EventConsumed::Yes;
			}
			break;
			}

			return EventConsumed::No;
		},

		[&](const sf::Event::MouseButtonReleased& event)
		{
			sf::Vector2f position = App::instance().window.mapPixelToCoords(event.position);
			switch (event.button)
			{
			case sf::Mouse::Button::Left:
			case sf::Mouse::Button::Right:
			{
				auto& game = App::instance().game;
				Vec2s pressedCell = game.getPressedCell();
				game.setPressedCell(INVALID_VEC2S);

				auto& board = game.getBoard();
				Vec2s coo{std::size_t(position.x), std::size_t(position.y)};

				// Only continue if we release the mouse on the previously pressed cell
				if (pressedCell != coo)
					return EventConsumed::Yes;

				if (event.button == sf::Mouse::Button::Right)
				{
					game.flag(coo);
					return EventConsumed::Yes;
				}

				game.open(coo);
				return EventConsumed::Yes;
			}
			break;

			case sf::Mouse::Button::Middle:
			{
				isMouseDraggingCamera_ = false;
				return EventConsumed::Yes;
			}
			break;
			}

			return EventConsumed::No;
		},

		[&](const sf::Event::MouseWheelScrolled& event)
		{
			constexpr float sensitivity = -0.1f;
			float factor = std::pow(2.f, event.delta * sensitivity);
			sf::View view = App::instance().window.getView();
			view.zoom(factor);
			App::instance().window.setView(view);
			return EventConsumed::Yes;
		},

		[](const auto&)
		{
			return EventConsumed::No;
		}
	};

	return event.visit(visitor);
}
