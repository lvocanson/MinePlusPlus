#include "MinesweeperLayer.h"
#include "App.h"
#include "Resources.h"
#include "SFMLExtensions.h"

using namespace SFMLExtensions;

MinesweeperLayer::MinesweeperLayer(Vec2s size, std::size_t mineCount)
	: pressedCellIdx_(-1)
	, moving_(false)
	, gameEnd_(false)
{
	board_.resize(size);
	board_.setMineCount(mineCount);
	board_.placeMines();
	clock_.reset();

	App::instance().setClearColor({0x79, 0x31, 0x32, 0x00});
	centerBoardOnView();
}

void MinesweeperLayer::centerBoardOnView()
{
	sf::Vector2f size = {float(board_.getSize().x), float(board_.getSize().y)};
	sf::Vector2f center = {float(size.x) * .5f, float(size.y) * .5f};

	auto windowSize = sf::Vector2f(App::instance().getWindowSize());
	float aspect = windowSize.x / windowSize.y;
	float verticalSize = std::max(size.x, size.y * aspect);

	sf::View view = App::instance().getView();
	view.setCenter(center);
	view.setSize({verticalSize * aspect, verticalSize});
	App::instance().setView(view);
}

EventConsumed MinesweeperLayer::listenEvent(const sf::Event& event)
{
	return event.visit(*this);
}

void MinesweeperLayer::render(sf::RenderTarget& target) const
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
		const sf::FloatRect rect{{float(x), float(y)}, {1.f, 1.f}};
		drawTexture(target, *texture, rect);
	}
}

EventConsumed MinesweeperLayer::operator()(const sf::Event::Closed& event)
{
	App::instance().exit();
	return EventConsumed::Yes;
}

EventConsumed MinesweeperLayer::operator()(const sf::Event::Resized& event)
{
	// Change the view aspect ratio to match the window
	auto size = sf::Vector2f(event.size);
	sf::View view = App::instance().getView();
	sf::Vector2f viewSize = view.getSize();
	float ratio = (viewSize.x * size.y) / (size.x * viewSize.y);
	viewSize.y *= ratio;
	view.setSize(viewSize);
	App::instance().setView(view);
	return EventConsumed::No;
}

EventConsumed MinesweeperLayer::operator()(const sf::Event::MouseMovedRaw& event)
{
	if (moving_)
	{
		constexpr float sensitivity = 1.f;
		sf::View view = App::instance().getView();
		auto windowSize = sf::Vector2f(App::instance().getWindowSize());
		// Assuming view and windowSize have the same aspect ratio
		float viewRatio = view.getSize().x / windowSize.x;
		auto offset = -(sensitivity * viewRatio) * sf::Vector2f(event.delta);
		view.move(offset);
		App::instance().setView(view);
	}
	return EventConsumed::No;
}

EventConsumed MinesweeperLayer::operator()(const sf::Event::MouseButtonPressed& event)
{
	switch (event.button)
	{
	case sf::Mouse::Button::Left:
	case sf::Mouse::Button::Right:
	{
		auto world = App::instance().screenToWorld(event.position);
		Vec2s coo{std::size_t(world.x), std::size_t(world.y)};
		if (board_.areCoordinatesValid(coo))
		{
			pressedCellIdx_ = board_.toIndex(coo);
			return EventConsumed::Yes;
		}
	}
	break;

	case sf::Mouse::Button::Middle:
	{
		moving_ = true;
		return EventConsumed::Yes;
	}
	break;
	}

	return EventConsumed::No;
}

EventConsumed MinesweeperLayer::operator()(const sf::Event::MouseButtonReleased& event)
{
	switch (event.button)
	{
	case sf::Mouse::Button::Left:
	case sf::Mouse::Button::Right:
	{
		auto pressedCellIdx = std::exchange(pressedCellIdx_, -1);
		auto world = App::instance().screenToWorld(event.position);
		Vec2s coo{std::size_t(world.x), std::size_t(world.y)};
		if (!board_.areCoordinatesValid(coo))
			return EventConsumed::No;

		// Only continue if we release the mouse on the previously pressed cell
		if (pressedCellIdx != board_.toIndex(coo))
			return EventConsumed::Yes;

		if (event.button == sf::Mouse::Button::Right)
		{
			board_.flag(pressedCellIdx);
			return EventConsumed::Yes;
		}

		if (!gameEnd_ && !clock_.isRunning()) // first click
		{
			board_.makeSafe(pressedCellIdx);
			clock_.restart();
		}

		bool lost = board_.open(pressedCellIdx);
		if (gameEnd_ = lost || board_.isWon())
		{
			clock_.stop();
		}

		return EventConsumed::Yes;
	}
	break;

	case sf::Mouse::Button::Middle:
	{
		moving_ = false;
		return EventConsumed::Yes;
	}
	break;
	}

	return EventConsumed::No;
}

EventConsumed MinesweeperLayer::operator()(const sf::Event::MouseWheelScrolled& event)
{
	constexpr float sensitivity = -0.1f;
	float factor = std::pow(2.f, event.delta * sensitivity);
	sf::View view = App::instance().getView();
	view.setSize(view.getSize() * factor);
	App::instance().setView(view);
	return EventConsumed::Yes;
}

EventConsumed MinesweeperLayer::operator()(const sf::Event::KeyPressed& event)
{
	if (event.code == sf::Keyboard::Key::Escape)
	{
		gameEnd_ = false;
		board_.clear();
		board_.placeMines();
		clock_.reset();
		return EventConsumed::Yes;
	}
	return EventConsumed::No;
}
