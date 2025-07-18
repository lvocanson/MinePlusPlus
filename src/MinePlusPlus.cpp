#include <SFML/Graphics.hpp>
#include <cassert>
#include <random>

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

struct Cell
{
	bool mined : 1;
	bool opened : 1;
	bool flagged : 1;
	std::uint8_t adjacentMines : 5; // [0, 31]
};

struct Board
{
	Board(std::size_t width, std::size_t height)
		: width(width), height(height)
		, cells(width* height, Cell{})
	{
		assert(cells.size() > 0 && "width and height must be non-zero numbers");
	}

	bool areCoordinatesValid(std::size_t col, std::size_t row) const
	{
		// coordinates are unsigned, no need to check >= 0
		return col < width && row < height;
	}

	std::size_t toIndex(std::size_t col, std::size_t row) const
	{
		assert(areCoordinatesValid(col, row));
		return col + row * width;
	}

	// call `func` up to 9 times with the cell at (col, row) and its neighbours
	template <typename F> requires requires (F f, Cell c, size_t i) { { f(c, i) }; }
	void foreachNeighboor(std::size_t col, std::size_t row, F&& func)
	{
		for (int dr = -1; dr <= 1; ++dr)
		{
			for (int dc = -1; dc <= 1; ++dc)
			{
				const std::size_t nCol = col + dc;
				const std::size_t nRow = row + dr;

				if (!areCoordinatesValid(nCol, nRow))
					continue;

				const auto nIdx = toIndex(nCol, nRow);
				func(cells[nIdx], nIdx);
			}
		}
	}

	// call `func` up to 9 times with the cell at index and its neighbours
	template <typename F> requires requires (F f, Cell c, size_t i) { { f(c, i) }; }
	void foreachNeighboor(std::size_t index, F&& func)
	{
		const auto col = index % width;
		const auto row = index / width;
		foreachNeighboor(col, row, std::forward<F>(func));
	}

	void addMines(std::size_t count)
	{
		std::mt19937_64 gen(std::random_device{}());
		std::uniform_int_distribution<std::size_t> dist(0, cells.size() - 1);

		for (std::size_t minesRemaining = count; minesRemaining > 0;)
		{
			const auto randIdx = dist(gen);
			auto& randCell = cells[randIdx];
			if (randCell.mined)
				continue;

			randCell.mined = true;
			foreachNeighboor(randIdx, [](Cell& c, auto) { ++c.adjacentMines; });
			--minesRemaining;
		}
	}

	void open(std::size_t index)
	{
		assert(index < cells.size());

		auto& first = cells[index];
		if (first.opened || first.flagged)
			return;

		std::vector<std::size_t> cellIndices = {index};
		while (!cellIndices.empty())
		{
			const auto idx = cellIndices.back();
			cellIndices.pop_back();

			auto& cell = cells[idx];
			cell.opened = true;
			if (cell.adjacentMines == 0)
			{
				foreachNeighboor(idx, [&](Cell& c, size_t i)
				{
					if (!c.opened) cellIndices.emplace_back(i);
				});
			}
		}
	}

	void flag(std::size_t index)
	{
		assert(index < cells.size());
		cells[index].flagged ^= true;
	}

	std::size_t width, height;
	std::vector<Cell> cells;
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

	sf::Sprite unopenedCellSprite{unopenedCellTexture};
	sf::Sprite unopenedSelectedCellSprite{unopenedSelectedCellTexture};
	sf::Sprite unopenedFlaggedCellSprite{unopenedFlaggedCellTexture};
	sf::Sprite openedCell0Sprite{openedCell0Texture};
	sf::Sprite openedCell1Sprite{openedCell1Texture};
	sf::Sprite openedCell2Sprite{openedCell2Texture};
	sf::Sprite openedCell3Sprite{openedCell3Texture};
	sf::Sprite openedCell4Sprite{openedCell4Texture};
	sf::Sprite openedCell5Sprite{openedCell5Texture};
	sf::Sprite openedCell6Sprite{openedCell6Texture};
	sf::Sprite openedCell7Sprite{openedCell7Texture};
	sf::Sprite openedCell8Sprite{openedCell8Texture};
	sf::Sprite openedCellClickedMineSprite{openedCellClickedMineTexture};
	sf::Sprite openedCellRunningMineSprite{openedCellRunningMineTexture};
	sf::Sprite openedCellNoMineSprite{openedCellNoMineTexture};
	sf::Sprite openedCellMineSprite{openedCellMineTexture};

	std::size_t boardWidth = 16;
	std::size_t boardHeight = 9;
	std::size_t minedCellsCount = 20;
	std::size_t cellSize = 64;

	Board board(boardWidth, boardHeight);
	board.addMines(minedCellsCount);

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
				std::size_t col = data->position.x / cellSize;
				std::size_t row = data->position.y / cellSize;
				if (board.areCoordinatesValid(col, row))
				{
					pressedCellIdx = board.toIndex(col, row);
				}
			}

			else if (auto* data = event->getIf<sf::Event::MouseButtonReleased>())
			{
				std::size_t col = data->position.x / cellSize;
				std::size_t row = data->position.y / cellSize;
				if (board.areCoordinatesValid(col, row))
				{
					if (pressedCellIdx == board.toIndex(col, row))
					{
						switch (data->button)
						{
						case sf::Mouse::Button::Left:
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
		}

		window.clear();


		for (std::size_t row = 0, idx = 0; row < boardHeight; ++row)
		{
			const auto y = row * cellSize;
			for (std::size_t col = 0; col < boardWidth; ++col, ++idx)
			{
				const auto& cell = board.cells[idx];
				const auto x = col * cellSize;
				const sf::Vector2f position = sf::Vector2f(sf::Vector2{x, y});

				auto drawSprite = [&](sf::Sprite& s) -> void
				{
					s.setPosition(position);
					window.draw(s);
				};

				// Check invalid state
				assert(!(cell.flagged && cell.opened));

				if (cell.flagged)
				{
					drawSprite(unopenedFlaggedCellSprite);
					continue;
				}

				if (!cell.opened)
				{
					if (pressedCellIdx == idx)
						drawSprite(unopenedSelectedCellSprite);
					else
						drawSprite(unopenedCellSprite);
					continue;
				}

				if (cell.mined)
				{
					drawSprite(openedCellMineSprite);
					continue;
				}

				switch (cell.adjacentMines)
				{
				case 0: drawSprite(openedCell0Sprite); continue;
				case 1: drawSprite(openedCell1Sprite); continue;
				case 2: drawSprite(openedCell2Sprite); continue;
				case 3: drawSprite(openedCell3Sprite); continue;
				case 4: drawSprite(openedCell4Sprite); continue;
				case 5: drawSprite(openedCell5Sprite); continue;
				case 6: drawSprite(openedCell6Sprite); continue;
				case 7: drawSprite(openedCell7Sprite); continue;
				case 8: drawSprite(openedCell8Sprite); continue;
				}

				// Invalid nb of adjacent mines
				assert(false);
			}
		}

		window.display();
	}
}
