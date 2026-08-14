#include "PlayMenu.h"
#include "Core/App.h"
#include "MainMenu.h"
#include "CustomMenu.h"
#include "Game/GameUI.h"
#include "UI/UITarget.h"

namespace
{

constexpr sf::Vector2i BUTTON_SIZE = {150, 28};
constexpr sf::Vector2i ELEMENTS_GAP = {200, 60};

constexpr std::string_view GAME_MODE_TITLE[PlayMenu::GameMode::Count] =
{
	"Default",
	"Spinning",
	"Running Bomb"
};

constexpr std::string_view GAME_MODE_DESC[PlayMenu::GameMode::Count] =
{
	"Default Minesweeper:\n"
	"Use the hints to discover all safe tiles!",

	"Spinning:\n"
	"Default Minesweeper, but the grid is slowly rotating on itself!",

	"Running Bomb:\n"
	"Default Minesweeper, but a random mine is a Running Bomb!\n"
	"The Running Bomb move at each revealing click.\n"
	"It moves only to an undiscovered tile,\n"
	"and can't move to a tile that already contains a mine."
};

}

PlayMenu::PlayMenu(Minesweeper& game)
	: gameMode_{}
	, gameModeText_{.origin = Text::Middle, .string = "Game Mode:"}
	, gameModeDescription_{.origin = Text::Top, .string = GAME_MODE_DESC[gameMode_]}
	, gameModeBtn_{.rect = {{}, BUTTON_SIZE}, .text = GAME_MODE_TITLE[gameMode_]}
	, beginnerBtn_{.rect = {{}, BUTTON_SIZE}, .text = "Beginner"}
	, intermediateBtn_{.rect = {{}, BUTTON_SIZE}, .text = "Intermediate"}
	, expertBtn_{.rect = {{}, BUTTON_SIZE}, .text = "Expert"}
	, backBtn_{.rect = {{}, BUTTON_SIZE}, .text = "Back"}
	, customBtn_{.rect = {{}, BUTTON_SIZE}, .text = "Custom"} {}

UIEvent::Result PlayMenu::operator()(const UIEvent::Resized& event)
{
	sf::Vector2i centerView = event.newSize / 2;
	gameModeText_.position = centerView + sf::Vector2i(gameModeBtn_.rect.size.x / -2, -ELEMENTS_GAP.y);
	gameModeBtn_.rect.position = centerView - gameModeBtn_.rect.size / 2 + sf::Vector2i(
		                             gameModeBtn_.rect.size.x / 2,
		                             -ELEMENTS_GAP.y);
	beginnerBtn_.rect.position = centerView - beginnerBtn_.rect.size / 2 - sf::Vector2i(ELEMENTS_GAP.x, 0);
	intermediateBtn_.rect.position = centerView - intermediateBtn_.rect.size / 2;
	expertBtn_.rect.position = centerView - expertBtn_.rect.size / 2 + sf::Vector2i(ELEMENTS_GAP.x, 0);
	backBtn_.rect.position = centerView - backBtn_.rect.size / 2 + sf::Vector2i(-ELEMENTS_GAP.x, ELEMENTS_GAP.y);
	customBtn_.rect.position = centerView - customBtn_.rect.size / 2 + ELEMENTS_GAP;
	gameModeDescription_.position = centerView + sf::Vector2i(0, ELEMENTS_GAP.y * 2);
	return UIEvent::Consumed;
}

UIEvent::Result PlayMenu::operator()(const UIEvent::Pressed& event)
{
	return tracker_.registerPress(
		       event.position,
		       {
			       gameModeBtn_,
			       beginnerBtn_,
			       intermediateBtn_,
			       expertBtn_,
			       backBtn_,
			       customBtn_
		       })
	       ? UIEvent::Consumed
	       : UIEvent::Ignored;
}

UIEvent::Result PlayMenu::operator()(const UIEvent::Released& event)
{
	if (tracker_.isClicked(gameModeBtn_, event.position))
	{
		gameMode_ = GameMode((gameMode_ + 1) % Count);
		gameModeDescription_.string = GAME_MODE_DESC[gameMode_];
		gameModeBtn_.text = GAME_MODE_TITLE[gameMode_];
	}
	else if (tracker_.isClicked(beginnerBtn_, event.position))
	{
		play(event.app, Easy);
	}
	else if (tracker_.isClicked(intermediateBtn_, event.position))
	{
		play(event.app, Medium);
	}
	else if (tracker_.isClicked(expertBtn_, event.position))
	{
		play(event.app, Hard);
	}
	else if (tracker_.isClicked(backBtn_, event.position))
	{
		event.app.submitCommand<SwapUI>(SwapUI::DEFAULT<MainMenu>);
	}
	else if (tracker_.isClicked(customBtn_, event.position))
	{
		event.app.submitCommand<SwapUI>([&game = event.app.getGame()](AppUI& ui) { ui.emplace<CustomMenu>(game); });
	}
	else
		return UIEvent::Ignored;
	return UIEvent::Consumed;
}

void PlayMenu::render(UITarget& target) const
{
	target.draw(gameModeText_);
	target.draw(gameModeDescription_);
	target.draw(gameModeBtn_);
	target.draw(beginnerBtn_);
	target.draw(intermediateBtn_);
	target.draw(expertBtn_);
	target.draw(backBtn_);
	target.draw(customBtn_);
}

void PlayMenu::play(App& app, Difficulty d) const
{
	auto& game = app.getGame();
	game.resetParameters();

	switch (d)
	{
	case PlayMenu::Easy: game.setEasy();
		break;
	case PlayMenu::Medium: game.setMedium();
		break;
	case PlayMenu::Hard: game.setHard();
		break;
	}

	game.restart();

	switch (gameMode_)
	{
	case PlayMenu::Spinning:
	{
		game.setRotationSpeed(1.f);
	}
	break;
	case PlayMenu::RunningBomb:
	{
		game.setRunningBombCount(d + 1ull);
	}
	break;
	}

	app.submitCommand<SwapUI>([&](AppUI& ui) { ui.emplace<GameUI>(app); });
	app.submitCommand<ChangeClearColor>(sf::Color{0x79, 0x31, 0x32, 0x00});
}
