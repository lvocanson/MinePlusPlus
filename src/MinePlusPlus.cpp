#include "Board.h"
#include <SFML/Graphics.hpp>
#include <cassert>

const std::filesystem::path RESOURCES_DIR = "res";
constexpr auto UNOPENED_CELL_FILE = "unopened-cell.png";
constexpr auto UNOPENED_SELECTED_CELL_FILE = "unopened-selected-cell.png";
constexpr auto UNOPENED_FLAGGED_CELL_FILE = "unopened-flagged-cell.png";
constexpr auto OPENED_CELL_0_FILE = "opened-cell-0.png";
constexpr auto OPENED_CELL_1_FILE = "opened-cell-1.png";
constexpr auto OPENED_CELL_2_FILE = "opened-cell-2.png";
constexpr auto OPENED_CELL_3_FILE = "opened-cell-3.png";
constexpr auto OPENED_CELL_4_FILE = "opened-cell-4.png";
constexpr auto OPENED_CELL_5_FILE = "opened-cell-5.png";
constexpr auto OPENED_CELL_6_FILE = "opened-cell-6.png";
constexpr auto OPENED_CELL_7_FILE = "opened-cell-7.png";
constexpr auto OPENED_CELL_8_FILE = "opened-cell-8.png";
constexpr auto OPENED_CELL_CLICKED_MINE_FILE = "opened-cell-clicked-mine.png";
constexpr auto OPENED_CELL_RUNNING_MINE_FILE = "opened-cell-running-mine.png";
constexpr auto OPENED_CELL_NO_MINE_FILE = "opened-cell-no-mine.png";
constexpr auto OPENED_CELL_MINE_FILE = "opened-cell-mine.png";
constexpr std::size_t TEXTURES_SIZE = 64;


void drawTexture(sf::RenderTarget& target, const sf::Texture& texture, sf::Vector2f position)
{
	assert(sf::Vector2<std::size_t>(texture.getSize()) == sf::Vector2(TEXTURES_SIZE, TEXTURES_SIZE));

	constexpr float size = TEXTURES_SIZE;
	sf::Vertex quad[] =
	{
		{
			.position = position,
			.texCoords = {0, 0}
		},
		{
			.position = position + sf::Vector2f(size, 0),
			.texCoords = {size, 0}
		},
		{
			.position = position + sf::Vector2f(size, size),
			.texCoords = {size, size}
		},
		{
			.position = position + sf::Vector2f(0, size),
			.texCoords = {0, size}
		}
	};

	auto states = sf::RenderStates::Default;
	states.texture = &texture;

	target.draw(quad, 4, sf::PrimitiveType::TriangleFan, states);
};

int main()
{
	auto window = sf::RenderWindow(sf::VideoMode({900u, 600u}), "Mine++");
	window.setVerticalSyncEnabled(true);

	const sf::Texture unopenedCellTexture{RESOURCES_DIR / UNOPENED_CELL_FILE};
	const sf::Texture unopenedSelectedCellTexture{RESOURCES_DIR / UNOPENED_SELECTED_CELL_FILE};
	const sf::Texture unopenedFlaggedCellTexture{RESOURCES_DIR / UNOPENED_FLAGGED_CELL_FILE};
	const sf::Texture openedCell0Texture{RESOURCES_DIR / OPENED_CELL_0_FILE};
	const sf::Texture openedCell1Texture{RESOURCES_DIR / OPENED_CELL_1_FILE};
	const sf::Texture openedCell2Texture{RESOURCES_DIR / OPENED_CELL_2_FILE};
	const sf::Texture openedCell3Texture{RESOURCES_DIR / OPENED_CELL_3_FILE};
	const sf::Texture openedCell4Texture{RESOURCES_DIR / OPENED_CELL_4_FILE};
	const sf::Texture openedCell5Texture{RESOURCES_DIR / OPENED_CELL_5_FILE};
	const sf::Texture openedCell6Texture{RESOURCES_DIR / OPENED_CELL_6_FILE};
	const sf::Texture openedCell7Texture{RESOURCES_DIR / OPENED_CELL_7_FILE};
	const sf::Texture openedCell8Texture{RESOURCES_DIR / OPENED_CELL_8_FILE};
	const sf::Texture openedCellClickedMineTexture{RESOURCES_DIR / OPENED_CELL_CLICKED_MINE_FILE};
	const sf::Texture openedCellRunningMineTexture{RESOURCES_DIR / OPENED_CELL_RUNNING_MINE_FILE};
	const sf::Texture openedCellNoMineTexture{RESOURCES_DIR / OPENED_CELL_NO_MINE_FILE};
	const sf::Texture openedCellMineTexture{RESOURCES_DIR / OPENED_CELL_MINE_FILE};

	Vec2s boardSize = {16, 9};
	std::size_t minedCellsCount = 20;

	Board board;
	board.resize(boardSize);

	std::size_t pressedCellIdx = -1;

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
				view.setSize(sf::Vector2f(data->size));
				window.setView(view);
			}

			else if (auto* data = event->getIf<sf::Event::MouseButtonPressed>())
			{
				auto coords = window.mapPixelToCoords(data->position, window.getView());
				Vec2s coo
				{
					.x = std::size_t(coords.x / TEXTURES_SIZE),
					.y = std::size_t(coords.y / TEXTURES_SIZE)
				};
				if (board.areCoordinatesValid(coo))
				{
					pressedCellIdx = board.toIndex(coo);
				}
			}

			else if (auto* data = event->getIf<sf::Event::MouseButtonReleased>())
			{
				auto coords = window.mapPixelToCoords(data->position, window.getView());
				Vec2s coo
				{
					.x = std::size_t(coords.x / TEXTURES_SIZE),
					.y = std::size_t(coords.y / TEXTURES_SIZE)
				};
				if (board.areCoordinatesValid(coo))
				{
					if (pressedCellIdx == board.toIndex(coo))
					{
						switch (data->button)
						{
						case sf::Mouse::Button::Left:
							if (board.getMineCount() == 0)
								board.placeMines(minedCellsCount, pressedCellIdx);
							board.open(pressedCellIdx);
							break;
						case sf::Mouse::Button::Right:
							board.flag(pressedCellIdx);
							break;
						}
					}
				}
				pressedCellIdx = -1;
			}

			else if (auto* data = event->getIf<sf::Event::KeyPressed>())
			{
				if (data->code == sf::Keyboard::Key::Escape)
					board.clear();
			}
		}

		window.clear();

		for (std::size_t row = 0, idx = 0; row < boardSize.y; ++row)
		{
			for (std::size_t col = 0; col < boardSize.x; ++col, ++idx)
			{
				auto& cell = board.getCellAt(idx);
				const sf::Vector2f position = sf::Vector2f(sf::Vector2{col, row} *TEXTURES_SIZE);

				if (cell.flagged)
				{
					drawTexture(window, unopenedFlaggedCellTexture, position);
					continue;
				}

				if (!cell.opened)
				{
					if (pressedCellIdx == idx)
						drawTexture(window, unopenedSelectedCellTexture, position);
					else
						drawTexture(window, unopenedCellTexture, position);
					continue;
				}

				if (cell.mined)
				{
					drawTexture(window, openedCellClickedMineTexture, position);
					continue;
				}

				switch (cell.adjacentMines)
				{
				case 0: drawTexture(window, openedCell0Texture, position); continue;
				case 1: drawTexture(window, openedCell1Texture, position); continue;
				case 2: drawTexture(window, openedCell2Texture, position); continue;
				case 3: drawTexture(window, openedCell3Texture, position); continue;
				case 4: drawTexture(window, openedCell4Texture, position); continue;
				case 5: drawTexture(window, openedCell5Texture, position); continue;
				case 6: drawTexture(window, openedCell6Texture, position); continue;
				case 7: drawTexture(window, openedCell7Texture, position); continue;
				case 8: drawTexture(window, openedCell8Texture, position); continue;
				}

				// Invalid nb of adjacent mines
				assert(false);
			}
		}

		window.display();
	}
}
