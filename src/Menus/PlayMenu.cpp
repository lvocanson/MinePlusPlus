#include "PlayMenu.h"
#include "Core/App.h"
#include "MainMenu.h"
#include "Game/BoardRenderer.h"
#include "Game/MinesweeperInput.h"
#include "Game/UserInterface.h"
#include "Game/Modes/SpinningMode.h"
#include "Game/Modes/RunningBombMode.h"

namespace
{

constexpr sf::Vector2i BUTTON_SIZE = {150, 28};
constexpr sf::Vector2i ELEMENTS_GAP = {200, 60};

enum GameMode : unsigned
{
	Default,
	Spinning,
	RunningBomb,
	Count
};

constexpr std::string_view GAME_MODE_TITLE[GameMode::Count] =
{
	"Default", "Spinning", "Running Bomb"
};

constexpr std::string_view GAME_MODE_DESC[GameMode::Count] =
{
	"Default Minesweeper:\nUse the hints to discover all safe tiles!",
	"Spinning:\nDefault Minesweeper, but the grid is slowly rotating on itself!",
	"Running Bomb:\nDefault Minesweeper, but a random mine is a Running Bomb!\nThe Running Bomb move at each revealing click.\nIt moves only to an undiscovered tile,\nand can't move to a tile that already contains a mine."
};

}

PlayMenu::PlayMenu()
	: gameMode_{}
	, gameModeText_{.origin = Text::Middle, .string = "Game Mode:"}
	, gameModeDescription_{.origin = Text::Top, .string = GAME_MODE_DESC[gameMode_]}
	, gameModeBtn_{.rect = {{}, BUTTON_SIZE}, .text = GAME_MODE_TITLE[gameMode_]}
	, beginnerBtn_{.rect = {{}, BUTTON_SIZE}, .text = "Beginner"}
	, intermediateBtn_{.rect = {{}, BUTTON_SIZE}, .text = "Intermediate"}
	, expertBtn_{.rect = {{}, BUTTON_SIZE}, .text = "Expert"}
	, backBtn_{.rect = {{}, BUTTON_SIZE}, .text = "Back"}
	, customBtn_{.rect = {{}, BUTTON_SIZE}, .text = "Custom"}
{
}

void PlayMenu::onScreenResized(sf::Vector2i newSize)
{
	sf::Vector2i center = newSize / 2;
	gameModeText_.position = center + sf::Vector2i(gameModeBtn_.rect.size.x / -2, -ELEMENTS_GAP.y);
	gameModeBtn_.rect.position = center - gameModeBtn_.rect.size / 2 + sf::Vector2i(gameModeBtn_.rect.size.x / 2, -ELEMENTS_GAP.y);
	beginnerBtn_.rect.position = center - beginnerBtn_.rect.size / 2 - sf::Vector2i(ELEMENTS_GAP.x, 0);
	intermediateBtn_.rect.position = center - intermediateBtn_.rect.size / 2;
	expertBtn_.rect.position = center - expertBtn_.rect.size / 2 + sf::Vector2i(ELEMENTS_GAP.x, 0);
	backBtn_.rect.position = center - backBtn_.rect.size / 2 + sf::Vector2i(-ELEMENTS_GAP.x, ELEMENTS_GAP.y);
	customBtn_.rect.position = center - customBtn_.rect.size / 2 + ELEMENTS_GAP;
	gameModeDescription_.position = center + sf::Vector2i(0, ELEMENTS_GAP.y * 2);
}

EventConsumed PlayMenu::onMouseButtonReleased(sf::Vector2i position)
{
	if (gameModeBtn_.rect.contains(position))
	{
		gameMode_ = (gameMode_ + 1) % GameMode::Count;
		gameModeDescription_.string = GAME_MODE_DESC[gameMode_];
		gameModeBtn_.text = GAME_MODE_TITLE[gameMode_];
	}
	else if (beginnerBtn_.rect.contains(position))
	{
		App::instance().game.setEasy();
		play();
	}
	else if (intermediateBtn_.rect.contains(position))
	{
		App::instance().game.setMedium();
		play();
	}
	else if (expertBtn_.rect.contains(position))
	{
		App::instance().game.setHard();
		play();
	}
	else if (backBtn_.rect.contains(position))
	{
		App::instance().layerStack.scheduleAsyncCommand<LayerStack::Swap>(this, std::make_unique<MainMenu>());
	}
	else if (customBtn_.rect.contains(position))
	{
		// TODO
	}
	else
		return EventConsumed::No;
	return EventConsumed::Yes;
}

void PlayMenu::play()
{
	auto& layerStack = App::instance().layerStack;
	layerStack.scheduleAsyncCommand<LayerStack::Swap>(this, std::make_unique<BoardRenderer>());
	layerStack.scheduleAsyncCommand<LayerStack::Push>(std::make_unique<MinesweeperInput>());
	layerStack.scheduleAsyncCommand<LayerStack::Push>(std::make_unique<UserInterface>());

	switch (gameMode_)
	{
	case GameMode::Spinning:
		layerStack.scheduleAsyncCommand<LayerStack::Push>(std::make_unique<SpinningMode>());
		break;
	case GameMode::RunningBomb:
		layerStack.scheduleAsyncCommand<LayerStack::Push>(std::make_unique<RunningBombMode>());
		break;
	}
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
