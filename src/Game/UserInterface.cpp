#include "UserInterface.h"
#include "Core/App.h"
#include "Core/WindowInput.h"
#include "Game/Resources.h"
#include "Game/MainMenu.h"
#include "Utils/Overloaded.h"
#include <format>

namespace
{

constexpr int SCREEN_PADDING = 5;
constexpr sf::Vector2i BUTTON_SIZE = {150, 28};

}

UserInterface::UserInterface()
	: restartBtn_{.rect = {{}, BUTTON_SIZE}, .text = "Restart"}
	, mainMenuBtn_{.rect = {{}, BUTTON_SIZE}, .text = "Main Menu"}
	, resetCameraBtn_{.rect = {{}, BUTTON_SIZE}, .text = "Reset Camera"}
{
	App::instance().clearColor = {0x79, 0x31, 0x32, 0x00};
	App::instance().game.setMedium();
	onScreenResized(sf::Vector2i(App::instance().window.getSize()));
	centerBoardOnView();
}

EventConsumed UserInterface::handleEvent(const sf::Event& event)
{
	if (auto* mbPressed = event.getIf<sf::Event::MouseButtonPressed>())
	{
		if (restartBtn_.rect.contains(mbPressed->position))
		{
			App::instance().game.restart();
		}
		else if (mainMenuBtn_.rect.contains(mbPressed->position))
		{
			App::instance().layerStack.scheduleAsyncCommand<LayerStack::Clear>();
			App::instance().layerStack.scheduleAsyncCommand<LayerStack::Push>(std::make_unique<WindowInput>());
			App::instance().layerStack.scheduleAsyncCommand<LayerStack::Push>(std::make_unique<MainMenu>());
		}
		else if (resetCameraBtn_.rect.contains(mbPressed->position))
		{
			centerBoardOnView();
		}
		else
			return EventConsumed::No;
		return EventConsumed::Yes;
	}

	return UserInterfaceLayer::handleEvent(event);
}

void UserInterface::centerBoardOnView()
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

void UserInterface::update(float dt)
{
	auto& board = App::instance().game.getBoard();
	std::make_signed_t<std::size_t> minesLeft = board.getMineCount() - board.getFlagCount();
	float bestTime = std::numeric_limits<float>::signaling_NaN(); // TODO
	auto timeInSeconds = App::instance().game.getPlayingTime().asMicroseconds() / 1'000'000;

	gameText_.string = std::format("Mines left: {}\nBest time: {}\nTime: {}s",
		minesLeft, bestTime, timeInSeconds);
}

void UserInterface::onScreenResized(sf::Vector2i newSize)
{
	gameText_.position =
	{
		SCREEN_PADDING,
		SCREEN_PADDING
	};

	restartBtn_.rect.position =
	{
		newSize.x - SCREEN_PADDING - restartBtn_.rect.size.x,
		SCREEN_PADDING
	};

	mainMenuBtn_.rect.position =
	{
		SCREEN_PADDING,
		newSize.y - SCREEN_PADDING - mainMenuBtn_.rect.size.y
	};

	resetCameraBtn_.rect.position =
	{
		newSize.x - SCREEN_PADDING - resetCameraBtn_.rect.size.x,
		newSize.y - SCREEN_PADDING - resetCameraBtn_.rect.size.y
	};
}

void UserInterface::render(UITarget& target) const
{
	target.draw(gameText_);
	target.draw(resetCameraBtn_);
	target.draw(mainMenuBtn_);
	target.draw(restartBtn_);
}
