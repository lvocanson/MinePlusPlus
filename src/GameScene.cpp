#include "GameScene.h"
#include "SFMLExtensions.h"

using namespace SFMLExtensions;

GameScene::GameScene(Vec2s size, std::size_t mineCount)
	: left_(FONT_REF)
	, center_(FONT_REF)
	, right_(FONT_REF)
	, pressedCellIdx_(-1)
	, gameEnd_(false)
{
	board_.resize(size);
	board_.setMineCount(mineCount);
	board_.placeMines();
	clock_.reset();

	left_.setCharacterSize(FONT_SIZE);
	left_.setFillColor(sf::Color::White);
	left_.setOutlineColor(sf::Color::Black);
	left_.setOutlineThickness(1.f);
	updateLeftText();
	{
		auto bounds = left_.getLocalBounds();
		left_.setOrigin({bounds.position.x - 1.f, 1.f});
	}

	center_ = left_;
	updateCenterText();
	{
		auto bounds = center_.getLocalBounds();
		center_.setOrigin({bounds.position.x + bounds.size.x / 2.f, 1.f});
	}

	right_ = left_;
	updateRightText();
	{
		auto bounds = right_.getLocalBounds();
		right_.setOrigin({bounds.position.x + bounds.size.x + 1.f, 1.f});
	}
}

void GameScene::onWindowResize(sf::Vector2u windowSize)
{
	sf::Vector2f availableBoardSize{float(windowSize.x), float(windowSize.y - 2 - FONT_SIZE)};

	auto& [boardCols, boardRows] = board_.getSize();
	sf::Vector2f cellScale = availableBoardSize.componentWiseDiv({float(boardCols), float(boardRows)});
	float uniformCellSize = std::min(cellScale.x, cellScale.y);

	sf::Vector2f boardPixelSize = {uniformCellSize * boardCols, uniformCellSize * boardRows};
	sf::Vector2f totalSceneSize = boardPixelSize + sf::Vector2f{0.f, 2 + FONT_SIZE};

	boardPosition_ = (sf::Vector2f(windowSize) - totalSceneSize) / 2.f;
	cellSize_ = {uniformCellSize, uniformCellSize};

	left_.setPosition(boardPosition_ + sf::Vector2f{0.f, boardPixelSize.y});
	center_.setPosition(boardPosition_ + sf::Vector2f{boardPixelSize.x / 2.f, boardPixelSize.y});
	right_.setPosition(boardPosition_ + boardPixelSize);
}

void GameScene::onMouseButtonPressed(sf::Vector2f coordinates, sf::Mouse::Button button)
{
	auto coo = ToBoardCoordinates(coordinates);
	if (board_.areCoordinatesValid(coo))
	{
		pressedCellIdx_ = board_.toIndex(coo);
	}
}

void GameScene::onMouseButtonReleased(sf::Vector2f coordinates, sf::Mouse::Button button)
{
	auto pressedCellIdx = std::exchange(pressedCellIdx_, -1);
	auto coo = ToBoardCoordinates(coordinates);
	if (!board_.areCoordinatesValid(coo))
		return;

	// Only continue if we release the mouse on the previously pressed cell
	if (pressedCellIdx != board_.toIndex(coo))
		return;

	switch (button)
	{
	case sf::Mouse::Button::Left:
	{
		if (!gameEnd_ && !clock_.isRunning()) // first click
		{
			board_.makeSafe(pressedCellIdx);
			clock_.restart();
		}

		bool lost = board_.open(pressedCellIdx);
		gameEnd_ = lost || board_.isWon();
		if (!gameEnd_)
			break;

		clock_.stop();
		if (lost)
			setStringAndKeepOrigin(center_, std::format("LOST!"));
		else
			setStringAndKeepOrigin(center_, std::format("VICTORY! {:.3f}", clock_.getElapsedTime().asSeconds()));
	}
	break;

	case sf::Mouse::Button::Right:
	{
		board_.flag(pressedCellIdx);
		updateLeftText();
	}
	break;
	}
}

void GameScene::onKeyPressed(sf::Keyboard::Key code)
{
	if (code == sf::Keyboard::Key::Escape)
	{
		gameEnd_ = false;
		board_.clear();
		board_.placeMines();
		clock_.reset();
		updateLeftText();
	}
}

void GameScene::update()
{
	if (!gameEnd_)
	{
		updateCenterText();
	}
}

void GameScene::drawOn(sf::RenderTarget& rt) const
{
	rt.clear({0x79, 0x31, 0x32, 0x00});

	for (std::size_t index = 0; index < board_.getCells().size(); ++index)
	{
		const sf::Texture* texture = nullptr;
		auto& cell = board_.getCellAt(index);

		if (cell.flagged)
		{
			texture = (gameEnd_ && !cell.mined)
				? &Resources::Textures::openedCellNoMine
				: &Resources::Textures::unopenedFlaggedCell;
		}

		else if (!cell.opened)
		{
			texture = (gameEnd_ && cell.mined)
				? &Resources::Textures::openedCellMine
				: (pressedCellIdx_ == index)
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

		auto [x, y] = board_.toCoordinates(index);
		const sf::FloatRect rect{boardPosition_ + cellSize_.componentWiseMul({float(x), float(y)}), cellSize_};
		drawTexture(rt, *texture, rect);
	}

	rt.draw(left_);
	rt.draw(center_);
	rt.draw(right_);
}

void GameScene::updateLeftText()
{
	std::make_signed_t<std::size_t> minesLeft = board_.getMineCount() - board_.getFlagCount(); // can be < 0
	setStringAndKeepOrigin(left_, std::format("Mines left: {}/{}", minesLeft, board_.getMineCount()));
}

void GameScene::updateCenterText()
{
	auto timeInSeconds = clock_.getElapsedTime().asMicroseconds() / 1'000'000;
	setStringAndKeepOrigin(center_, std::format("{}", timeInSeconds));
}

void GameScene::updateRightText()
{
	float bestTime = std::numeric_limits<float>::signaling_NaN(); // TODO
	setStringAndKeepOrigin(right_, std::format("Best: {:.3f}", bestTime));
}

Vec2s GameScene::ToBoardCoordinates(sf::Vector2f coordinates) const
{
	coordinates -= boardPosition_;
	auto [x, y] = coordinates.componentWiseDiv(cellSize_);
	return {std::size_t(x), std::size_t(y)};
}

sf::Vector2f GameScene::ToWorldCoordinates(const Vec2s& coordinates) const
{
	auto world = cellSize_.componentWiseMul({float(coordinates.x), float(coordinates.y)});
	world += boardPosition_;
	return world;
}
