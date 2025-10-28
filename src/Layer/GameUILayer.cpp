#include "GameUILayer.h"
#include "Board.h"
#include "Event/EventBus.h"
#include "Event/MinesweeperEvents.h"
#include <format>
#include <App.h>

GameUILayer::GameUILayer()
	: board_{}
	, pressedCellIdx_(-1)
{
	isMouseWheelControllingZoom_ = true;
	clock_.reset();

	App::instance().setClearColor({0x79, 0x31, 0x32, 0x00});

	EventBus::publish(MinesweeperEvents::SetRenderMode{MinesweeperEvents::RenderMode::Classic});
	EventBus::publish(MinesweeperEvents::BoardRequest{board_});

	if (board_)
	{
		*board_ = Board::MediumBoard();
		board_->placeMines();
		centerBoardOnView();
	}
}

EventConsumed GameUILayer::onPress(sf::Mouse::Button button, sf::Vector2f position)
{
	switch (button)
	{
	case sf::Mouse::Button::Left:
	case sf::Mouse::Button::Right:
	{
		if (!board_)
			return EventConsumed::No;

		Vec2s coo{std::size_t(position.x), std::size_t(position.y)};
		if (board_->areCoordinatesValid(coo))
		{
			pressedCellIdx_ = board_->toIndex(coo);
			EventBus::publish(MinesweeperEvents::SetSelectedCell{pressedCellIdx_});
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
		if (!board_)
			return EventConsumed::No;

		auto pressedCellIdx = std::exchange(pressedCellIdx_, -1);
		EventBus::publish(MinesweeperEvents::SetSelectedCell{pressedCellIdx_});

		Vec2s coo{std::size_t(position.x), std::size_t(position.y)};
		if (!board_->areCoordinatesValid(coo))
			return EventConsumed::No;

		// Only continue if we release the mouse on the previously pressed cell
		if (pressedCellIdx != board_->toIndex(coo))
			return EventConsumed::Yes;

		if (button == sf::Mouse::Button::Right)
		{
			board_->flag(pressedCellIdx);
			return EventConsumed::Yes;
		}

		if (board_->getOpenCount() == 0) // first click
		{
			board_->makeSafe(pressedCellIdx);
			clock_.restart();
		}

		bool lost = board_->open(pressedCellIdx);
		if (lost || board_->isWon())
		{
			EventBus::publish(MinesweeperEvents::SetRenderMode{MinesweeperEvents::RenderMode::RevealAll});
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
	if (board_)
	{
		board_->clear();
		board_->placeMines();
	}
	clock_.reset();
}

void GameUILayer::centerBoardOnView()
{
	if (!board_)
		return;

	sf::Vector2f size = {float(board_->getSize().x), float(board_->getSize().y)};
	sf::Vector2f center = {float(size.x) * .5f, float(size.y) * .5f};

	auto windowSize = sf::Vector2f(App::instance().getWindowSize());
	float aspect = windowSize.x / windowSize.y;
	float verticalSize = std::max(size.x, size.y * aspect);

	sf::View view = App::instance().getView();
	view.setCenter(center);
	view.setSize({verticalSize * aspect, verticalSize});
	App::instance().setView(view);
}


void GameUILayer::render(sf::RenderTarget& target) const
{
	std::make_signed_t<std::size_t> minesLeft = board_
		? board_->getMineCount() - board_->getFlagCount() : 0;
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

	text.setPosition(viewCenter - viewSize * .5f);
	text.setScale(viewSize.componentWiseDiv(targetSize));

	target.draw(text);
}
