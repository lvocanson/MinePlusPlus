#include "GameScene.h"

int main()
{
	auto window = sf::RenderWindow(sf::VideoMode({900u, 600u}), "Mine++");
	window.setVerticalSyncEnabled(true);

	Vec2s boardSize = {9, 9}; std::size_t minedCellsCount = 10;
	//Vec2s boardSize = {16, 16}; std::size_t minedCellsCount = 40;
	//Vec2s boardSize = {30, 16}; std::size_t minedCellsCount = 99;

	GameScene game(boardSize, minedCellsCount);
	game.onWindowResize(window.getSize());

	while (window.isOpen())
	{
		while (const auto event = window.pollEvent())
		{
			if (event->is<sf::Event::Closed>())
			{
				window.close();
			}

			else if (auto* data = event->getIf<sf::Event::Resized>())
			{
				sf::View view = window.getView();
				sf::Vector2f size(data->size);
				view.setSize(size);
				view.setCenter(size / 2.f);
				window.setView(view);

				game.onWindowResize(data->size);
			}

			else if (auto* data = event->getIf<sf::Event::MouseButtonPressed>())
			{
				auto coords = window.mapPixelToCoords(data->position, window.getView());
				game.onMouseButtonPressed(coords, data->button);
			}

			else if (auto* data = event->getIf<sf::Event::MouseButtonReleased>())
			{
				auto coords = window.mapPixelToCoords(data->position, window.getView());
				game.onMouseButtonReleased(coords, data->button);
			}

			else if (auto* data = event->getIf<sf::Event::KeyPressed>())
			{
				game.onKeyPressed(data->code);
			}
		}

		window.clear();
		game.drawOn(window);
		window.display();
	}
}
