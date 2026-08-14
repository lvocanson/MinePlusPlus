#include "GameUI.h"
#include "Core/App.h"
#include "Game/Resources.h"
#include "Menus/MainMenu.h"
#include "UI/UITarget.h"
#include <format>
#include <iterator>

namespace
{

constexpr int SCREEN_PADDING = 5;
constexpr sf::Vector2i BUTTON_SIZE = {150, 28};

}

GameUI::GameUI(App& app)
	: game_(app.getGame())
	, restartBtn_{.rect = {{}, BUTTON_SIZE}, .text = "Restart"}
	, mainMenuBtn_{.rect = {{}, BUTTON_SIZE}, .text = "Main Menu"}
	, resetCameraBtn_{.rect = {{}, BUTTON_SIZE}, .text = "Reset Camera"}
{
	game_.setRendering(true);
	centerBoardOnView(app);
}

GameUI::~GameUI()
{
	game_.setRendering(false);
}

UIEvent::Result GameUI::operator()(const UIEvent::Resized& event)
{
	gameText_.position =
	{
		SCREEN_PADDING,
		SCREEN_PADDING
	};

	restartBtn_.rect.position =
	{
		event.newSize.x - SCREEN_PADDING - restartBtn_.rect.size.x,
		SCREEN_PADDING
	};

	mainMenuBtn_.rect.position =
	{
		SCREEN_PADDING,
		event.newSize.y - SCREEN_PADDING - mainMenuBtn_.rect.size.y
	};

	resetCameraBtn_.rect.position =
	{
		event.newSize.x - SCREEN_PADDING - resetCameraBtn_.rect.size.x,
		event.newSize.y - SCREEN_PADDING - resetCameraBtn_.rect.size.y
	};

	return UIEvent::Consumed;
}

UIEvent::Result GameUI::operator()(const UIEvent::Pressed& event)
{
	return tracker_.registerPress(
		       event.position,
		       {
			       restartBtn_,
			       mainMenuBtn_,
			       resetCameraBtn_
		       })
	       ? UIEvent::Consumed
	       : UIEvent::Ignored;
}

UIEvent::Result GameUI::operator()(const UIEvent::Released& event)
{
	if (tracker_.isClicked(restartBtn_, event.position))
	{
		event.app.getGame().restart();
	}
	else if (tracker_.isClicked(mainMenuBtn_, event.position))
	{
		event.app.submitCommand<SwapUI>(SwapUI::DEFAULT<MainMenu>);
		event.app.submitCommand<ChangeClearColor>(sf::Color{0x31, 0x4D, 0x79, 0x00});
	}
	else if (tracker_.isClicked(resetCameraBtn_, event.position))
	{
		centerBoardOnView(event.app);
	}
	else
		return UIEvent::Ignored;
	return UIEvent::Consumed;
}

void GameUI::render(UITarget& target) const
{
	auto& board = game_.getBoard();
	std::make_signed_t<std::size_t> minesLeft = board.getMineCount() - board.getFlagCount();
	float bestTime = std::numeric_limits<float>::signaling_NaN(); // TODO
	auto time = game_.getPlayingTime().asMicroseconds() / 1'000'000;

	gameString_.clear();
	std::format_to(
		std::back_inserter(gameString_),
		"Mines left: {}\nBest time: {}s\nTime: {}s",
		minesLeft,
		bestTime,
		time);
	gameText_.string = gameString_;

	target.draw(gameText_);
	target.draw(resetCameraBtn_);
	target.draw(mainMenuBtn_);
	target.draw(restartBtn_);
}

void GameUI::centerBoardOnView(App& app) const
{
	auto& board = game_.getBoard();
	sf::Vector2f size = {float(board.getSize().x), float(board.getSize().y)};
	sf::Vector2f margin = {1.f, 1.f};
	app.centerView({-margin, size + margin * 2.f});
}
