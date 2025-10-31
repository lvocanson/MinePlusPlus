#include "GameUILayer.h"
#include "App.h"
#include "Event/MinesweeperEvents.h"
#include <format>

GameUILayer::GameUILayer()
	: pressedCellIdx_(-1)
{
	isMouseWheelControllingZoom_ = true;
	clock_.reset();

	App::instance().clearColor = {0x79, 0x31, 0x32, 0x00};
	auto& board = App::instance().board;
	board = Board::MediumBoard();
	board.placeMines();
	centerBoardOnView();

	App::instance().eventBus.publish(MinesweeperEvents::SetRenderMode{MinesweeperEvents::RenderMode::Classic});
}

EventConsumed GameUILayer::onPress(sf::Mouse::Button button, sf::Vector2f position)
{
	switch (button)
	{
	case sf::Mouse::Button::Left:
	case sf::Mouse::Button::Right:
	{
		auto& board = App::instance().board;
		Vec2s coo{std::size_t(position.x), std::size_t(position.y)};
		if (board.areCoordinatesValid(coo))
		{
			pressedCellIdx_ = board.toIndex(coo);
			App::instance().eventBus.publish(MinesweeperEvents::SetSelectedCell{pressedCellIdx_});
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
}

EventConsumed GameUILayer::onRelease(sf::Mouse::Button button, sf::Vector2f position)
{
	switch (button)
	{
	case sf::Mouse::Button::Left:
	case sf::Mouse::Button::Right:
	{
		auto pressedCellIdx = std::exchange(pressedCellIdx_, -1);
		App::instance().eventBus.publish(MinesweeperEvents::SetSelectedCell{pressedCellIdx_});

		auto& board = App::instance().board;
		Vec2s coo{std::size_t(position.x), std::size_t(position.y)};
		if (!board.areCoordinatesValid(coo))
			return EventConsumed::No;

		// Only continue if we release the mouse on the previously pressed cell
		if (pressedCellIdx != board.toIndex(coo))
			return EventConsumed::Yes;

		if (button == sf::Mouse::Button::Right)
		{
			board.flag(pressedCellIdx);
			return EventConsumed::Yes;
		}

		if (board.getOpenCount() == 0) // first click
		{
			board.makeSafe(pressedCellIdx);
			clock_.restart();
		}

		bool lost = board.open(pressedCellIdx);
		if (lost || board.isWon())
		{
			App::instance().eventBus.publish(MinesweeperEvents::SetRenderMode{MinesweeperEvents::RenderMode::RevealAll});
			clock_.stop();
		}
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
}

void GameUILayer::restart()
{
	auto& board = App::instance().board;
	board.clear();
	board.placeMines();
	clock_.reset();
}

void GameUILayer::centerBoardOnView()
{
	auto& board = App::instance().board;
	sf::Vector2f size = {float(board.getSize().x), float(board.getSize().y)};
	sf::Vector2f center = {float(size.x) * .5f, float(size.y) * .5f};

	auto windowSize = sf::Vector2f(App::instance().window.getSize());
	float aspect = windowSize.x / windowSize.y;
	float verticalSize = std::max(size.x, size.y * aspect);

	sf::View view = App::instance().window.getView();
	view.setCenter(center);
	view.setSize({verticalSize * aspect, verticalSize});
	App::instance().window.setView(view);
}


void GameUILayer::render(sf::RenderTarget& target) const
{
	auto& board = App::instance().board;
	std::make_signed_t<std::size_t> minesLeft = board.getMineCount() - board.getFlagCount();
	float bestTime = std::numeric_limits<float>::signaling_NaN(); // TODO
	auto timeInSeconds = clock_.getElapsedTime().asMicroseconds() / 1'000'000;
	auto str = std::format("Mines left: {}\nBest time: {}\nTime: {}s",
		minesLeft, bestTime, timeInSeconds);

	sf::Text text(FONT_REF, str, FONT_SIZE);
	text.setFillColor(sf::Color::White);
	text.setOutlineColor(sf::Color::Black);
	text.setOutlineThickness(1.f);

	const sf::View& view = target.getView();
	sf::Vector2f viewCenter = view.getCenter();
	sf::Vector2f viewSize = view.getSize();
	auto targetSize = sf::Vector2f(target.getSize());

	text.setPosition(target.mapPixelToCoords({0, 0}));
	text.setRotation(view.getRotation());
	text.setScale(viewSize.componentWiseDiv(targetSize));

	target.draw(text);
}
