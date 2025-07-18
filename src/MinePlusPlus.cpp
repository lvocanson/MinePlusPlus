#include <SFML/Graphics.hpp>
#include <cassert>
#include <random>

struct Cell
{
	bool mined : 1;
	bool opened : 1;
	bool flagged : 1;
	std::uint8_t adjacentMines : 5; // [0, 31]

	bool open() { if (!flagged) opened = true; return mined && opened; }
	void flag() { if (!opened) flagged = !flagged; }
};

int main()
{
	auto window = sf::RenderWindow(sf::VideoMode({900u, 600u}), "Mine++");
	window.setVerticalSyncEnabled(true);

	const std::filesystem::path resourcesDir = "res";
	const auto unopenedCellPath = resourcesDir / "unopened-cell.png";
	const auto flaggedCellPath = resourcesDir / "flagged-cell.png";
	const auto openedCell0Path = resourcesDir / "opened-cell-0.png";
	const auto openedCell1Path = resourcesDir / "opened-cell-1.png";
	const auto openedCell2Path = resourcesDir / "opened-cell-2.png";
	const auto openedCell3Path = resourcesDir / "opened-cell-3.png";
	const auto openedCell4Path = resourcesDir / "opened-cell-4.png";
	const auto openedCell5Path = resourcesDir / "opened-cell-5.png";
	const auto openedCell6Path = resourcesDir / "opened-cell-6.png";
	const auto openedCell7Path = resourcesDir / "opened-cell-7.png";
	const auto openedCell8Path = resourcesDir / "opened-cell-8.png";
	const auto openedCellClickedMinePath = resourcesDir / "opened-cell-clicked-mine.png";
	const auto openedCellRunningMinePath = resourcesDir / "opened-cell-running-mine.png";
	const auto openedCellNoMinePath = resourcesDir / "opened-cell-no-mine.png";
	const auto openedCellMinePath = resourcesDir / "opened-cell-mine.png";

	sf::Texture unopenedCellTexture{unopenedCellPath};
	sf::Texture flaggedCellTexture{flaggedCellPath};
	sf::Texture openedCell0Texture{openedCell0Path};
	sf::Texture openedCell1Texture{openedCell1Path};
	sf::Texture openedCell2Texture{openedCell2Path};
	sf::Texture openedCell3Texture{openedCell3Path};
	sf::Texture openedCell4Texture{openedCell4Path};
	sf::Texture openedCell5Texture{openedCell5Path};
	sf::Texture openedCell6Texture{openedCell6Path};
	sf::Texture openedCell7Texture{openedCell7Path};
	sf::Texture openedCell8Texture{openedCell8Path};
	sf::Texture openedCellClickedMineTexture{openedCellClickedMinePath};
	sf::Texture openedCellRunningMineTexture{openedCellRunningMinePath};
	sf::Texture openedCellNoMineTexture{openedCellNoMinePath};
	sf::Texture openedCellMineTexture{openedCellMinePath};

	sf::Sprite unopenedCellSprite{unopenedCellTexture};
	sf::Sprite flaggedCellSprite{flaggedCellTexture};
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

	std::size_t boardWidth = 9;
	std::size_t boardHeight = 9;
	std::size_t minedCellsCount = 10;
	std::size_t cellSize = 64;

	std::vector<Cell> board(boardWidth * boardHeight, Cell{});
	sf::IntRect boardRect = {sf::Vector2i{}, sf::Vector2i(sf::Vector2{boardWidth * cellSize, boardHeight * cellSize})};

	assert(board.size() > 0);
	std::mt19937_64 gen(std::random_device{}());
	std::uniform_int_distribution<std::size_t> dist(0, board.size() - 1);

	for (std::size_t minesRemaining = minedCellsCount; minesRemaining > 0;)
	{
		const auto randIdx = dist(gen);
		auto& randCell = board[randIdx];
		if (randCell.mined)
			continue;

		randCell.mined = true;
		--minesRemaining;

		const auto row = randIdx / boardHeight;
		const auto col = randIdx % boardHeight;

		for (int dr = -1; dr <= 1; ++dr)
		{
			for (int dc = -1; dc <= 1; ++dc)
			{
				// coordinates are unsigned: underflow if (row + dr) < 0
				const std::size_t nRow = row + dr;
				const std::size_t nCol = col + dc;

				if (nRow >= boardHeight || nCol >= boardWidth)
					continue;

				const auto neighborIdx = nRow * boardWidth + nCol;
				board[neighborIdx].adjacentMines += 1;
			}
		}
	}

	Cell* pressedCell{};

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
				if (boardRect.contains(data->position))
				{
					std::size_t row = data->position.y / cellSize;
					std::size_t col = data->position.x / cellSize;
					pressedCell = &board[row * boardWidth + col];
				}
			}

			else if (auto* data = event->getIf<sf::Event::MouseButtonReleased>())
			{
				if (pressedCell && boardRect.contains(data->position))
				{
					std::size_t row = data->position.y / cellSize;
					std::size_t col = data->position.x / cellSize;
					
					if (pressedCell == &board[row * boardWidth + col])
					{
						switch (data->button)
						{
						case sf::Mouse::Button::Left:
							pressedCell->open();
							break;
						case sf::Mouse::Button::Right:
							pressedCell->flag();
							break;
						}
					}
				}
				pressedCell = nullptr;
			}
		}

		window.clear();


		for (std::size_t row = 0, idx = 0; row < boardHeight; ++row)
		{
			const auto y = row * cellSize;
			for (std::size_t col = 0; col < boardWidth; ++col, ++idx)
			{
				const auto& cell = board[idx];
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
					drawSprite(flaggedCellSprite);
					continue;
				}

				if (!cell.opened)
				{
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
