#include "GameUILayer.h"
#include "App.h"
#include <format>

GameUILayer::GameUILayer()
{
	isMouseWheelControllingZoom_ = true;

	App::instance().clearColor = {0x79, 0x31, 0x32, 0x00};
	App::instance().game.setMedium();
	centerBoardOnView();
}

EventConsumed GameUILayer::onPress(sf::Mouse::Button button, sf::Vector2f position)
{
	switch (button)
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
}

EventConsumed GameUILayer::onRelease(sf::Mouse::Button button, sf::Vector2f position)
{
	switch (button)
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

		if (button == sf::Mouse::Button::Right)
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
}

void GameUILayer::restart()
{
	App::instance().game.restart();
}

void GameUILayer::centerBoardOnView()
{
	auto& board = App::instance().game.getBoard();
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
	auto& board = App::instance().game.getBoard();
	std::make_signed_t<std::size_t> minesLeft = board.getMineCount() - board.getFlagCount();
	float bestTime = std::numeric_limits<float>::signaling_NaN(); // TODO
	auto timeInSeconds = App::instance().game.getPlayingTime().asMicroseconds() / 1'000'000;
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
