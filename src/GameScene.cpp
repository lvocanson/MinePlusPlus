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

	float bestTime = std::numeric_limits<float>::signaling_NaN();

	left_.setString(std::format("Mines left: 0/{}", mineCount));
	left_.setCharacterSize(FONT_SIZE);
	left_.setFillColor(sf::Color::White);
	left_.setOutlineColor(sf::Color::Black);
	left_.setOutlineThickness(1.f);
	left_.setOrigin({0.f, -1.f});

	center_ = left_;
	center_.setString("0");
	center_.setOrigin({center_.getLocalBounds().size.x / 2.f, -1.f});

	right_ = left_;
	right_.setString(std::format("Best: {:.3f}", bestTime));
	right_.setOrigin({right_.getLocalBounds().size.x, -1.f});
}

void GameScene::onWindowResize(sf::Vector2u size)
{
	sf::Vector2f boardArea{size};
	boardArea.y -= 2 + FONT_SIZE;
	auto [w, h] = board_.getSize();
	auto [x, y] = boardArea.componentWiseDiv({float(w), float(h)});
	float min = std::min(x, y);

	boardArea = {min * w, min * h};
	cellSize_ = {min, min};

	left_.setPosition({0.f, boardArea.y});
	center_.setPosition({boardArea.x / 2.f, boardArea.y});
	right_.setPosition(boardArea);
}

void GameScene::onMouseButtonPressed(sf::Vector2f coordinates, sf::Mouse::Button button)
{
	auto [x, y] = coordinates.componentWiseDiv(cellSize_);
	Vec2s coo = {std::size_t(x), std::size_t(y)};
	if (board_.areCoordinatesValid(coo))
	{
		pressedCellIdx_ = board_.toIndex(coo);
	}
}

void GameScene::onMouseButtonReleased(sf::Vector2f coordinates, sf::Mouse::Button button)
{
	auto [x, y] = coordinates.componentWiseDiv(cellSize_);
	Vec2s coo = {std::size_t(x), std::size_t(y)};
	if (board_.areCoordinatesValid(coo))
	{
		if (pressedCellIdx_ == board_.toIndex(coo))
		{
			switch (button)
			{
			case sf::Mouse::Button::Left:
			{
				if (!gameEnd_ && !clock_.isRunning()) // first click
				{
					board_.makeSafe(pressedCellIdx_);
					clock_.restart();
				}

				bool lost = board_.open(pressedCellIdx_);
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
				board_.flag(pressedCellIdx_);
				setStringAndKeepOrigin(left_, std::format("Mines left: {}/{}", board_.getFlagCount(), board_.getMineCount()));
			}
			break;
			}
		}
	}
	pressedCellIdx_ = -1;
}

void GameScene::onKeyPressed(sf::Keyboard::Key code)
{
	if (code == sf::Keyboard::Key::Escape)
	{
		gameEnd_ = false;
		board_.clear();
		board_.placeMines();
		clock_.reset();
		setStringAndKeepOrigin(left_, std::format("Mines left: 0/{}", board_.getMineCount()));
	}
}

void GameScene::drawOn(sf::RenderTarget& rt) const
{
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
		const sf::FloatRect rect{cellSize_.componentWiseMul({float(x), float(y)}), cellSize_};
		drawTexture(rt, *texture, rect);
	}

	if (!gameEnd_)
		setStringAndKeepOrigin(const_cast<sf::Text&>(center_), std::format("{}", clock_.getElapsedTime().asMilliseconds() / 1000));

	rt.draw(left_);
	rt.draw(center_);
	rt.draw(right_);
}
