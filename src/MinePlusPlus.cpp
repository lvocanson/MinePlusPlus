#include <SFML/Graphics.hpp>
#include <cassert>
#include <random>

struct Cell
{
	bool mined : 1;
	bool opened : 1;
	std::uint8_t adjacentMines : 6;
};

int main()
{
	auto window = sf::RenderWindow(sf::VideoMode({900u, 600u}), "Mine++");
	window.setVerticalSyncEnabled(true);

	std::filesystem::path resourcesDir = "res";
	auto unopenedCellPath = resourcesDir / "unopened-cell.png";
	auto flaggedCellPath = resourcesDir / "flagged-cell.png";
	auto openedCell0Path = resourcesDir / "opened-cell-0.png";
	auto openedCell1Path = resourcesDir / "opened-cell-1.png";
	auto openedCell2Path = resourcesDir / "opened-cell-2.png";
	auto openedCell3Path = resourcesDir / "opened-cell-3.png";
	auto openedCell4Path = resourcesDir / "opened-cell-4.png";
	auto openedCell5Path = resourcesDir / "opened-cell-5.png";
	auto openedCell6Path = resourcesDir / "opened-cell-6.png";
	auto openedCell7Path = resourcesDir / "opened-cell-7.png";
	auto openedCell8Path = resourcesDir / "opened-cell-8.png";
	auto openedCellClickedMinePath = resourcesDir / "opened-cell-clicked-mine.png";
	auto openedCellRunningMinePath = resourcesDir / "opened-cell-running-mine.png";
	auto openedCellNoMinePath = resourcesDir / "opened-cell-no-mine.png";
	auto openedCellMinePath = resourcesDir / "opened-cell-mine.png";

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
	float cellSize = 64.f;

	std::vector<Cell> board(boardWidth * boardHeight, Cell{});

	assert(board.size() > 0);
	std::random_device rd;
	std::mt19937_64 gen(rd());
	std::uniform_int_distribution<std::size_t> dist(0, board.size() - 1);

	for (std::size_t minesRemaining = minedCellsCount; minesRemaining > 0;)
	{
		auto randIdx = dist(gen);
		auto& randCell = board[randIdx];
		if (randCell.mined)
			continue;

		randCell.mined = true;
		--minesRemaining;

		auto row = randIdx / boardHeight;
		auto col = randIdx % boardHeight;

		for (int dr = -1; dr <= 1; ++dr)
		{
			for (int dc = -1; dc <= 1; ++dc)
			{
				std::size_t nRow = row + dr;
				std::size_t nCol = col + dc;

				if (nRow >= boardHeight || nCol >= boardWidth)
					continue;

				std::size_t neighborIdx = nRow * boardWidth + nCol;
				board[neighborIdx].adjacentMines += 1;
			}
		}
	}


	while (window.isOpen())
	{
		while (const auto event = window.pollEvent())
		{
			if (event->is<sf::Event::Closed>())
			{
				window.close();
			}
		}

		window.clear();

		for (std::size_t row = 0, idx = 0; row < boardHeight; ++row)
		{
			float y = row * cellSize;
			for (std::size_t col = 0; col < boardWidth; ++col, ++idx)
			{
				auto& cell = board[idx];
				float x = col * cellSize;

				if (cell.mined)
				{
					openedCellMineSprite.setPosition({x, y});
					window.draw(openedCellMineSprite);
					continue;
				}

				switch (cell.adjacentMines)
				{
				case 0:
					openedCell0Sprite.setPosition({x, y});
					window.draw(openedCell0Sprite);
					continue;
				case 1:
					openedCell1Sprite.setPosition({x, y});
					window.draw(openedCell1Sprite);
					continue;
				case 2:
					openedCell2Sprite.setPosition({x, y});
					window.draw(openedCell2Sprite);
					continue;
				case 3:
					openedCell3Sprite.setPosition({x, y});
					window.draw(openedCell3Sprite);
					continue;
				case 4:
					openedCell4Sprite.setPosition({x, y});
					window.draw(openedCell4Sprite);
					continue;
				case 5:
					openedCell5Sprite.setPosition({x, y});
					window.draw(openedCell5Sprite);
					continue;
				case 6:
					openedCell6Sprite.setPosition({x, y});
					window.draw(openedCell6Sprite);
					continue;
				case 7:
					openedCell7Sprite.setPosition({x, y});
					window.draw(openedCell7Sprite);
					continue;
				case 8:
					openedCell8Sprite.setPosition({x, y});
					window.draw(openedCell8Sprite);
					continue;
				}
			}
		}

		window.display();
	}
}
