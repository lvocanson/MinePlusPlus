#include "PlayMenu.h"
#include "Core/App.h"
#include "MainMenu.h"
#include "Game/BoardRenderer.h"
#include "Game/MinesweeperInput.h"
#include "Game/UserInterface.h"

namespace
{

constexpr sf::Vector2i BUTTON_SIZE = {150, 28};
constexpr sf::Vector2i ELEMENTS_GAP = {200, 60};

constexpr std::string_view DEFAULT_GAME_MODE_TITLE = "Default";
constexpr std::string_view SPINNING_GAME_MODE_TITLE = "Spinning";
constexpr std::string_view RUNBOMB_GAME_MODE_TITLE = "Running Bomb";

constexpr std::string_view DEFAULT_GAME_MODE_DESC = "Default Minesweeper:\nUse the hints to discover all safe tiles!";
constexpr std::string_view SPINNING_GAME_MODE_DESC = "Spinning:\nDefault Minesweeper, but the grid is slowly rotating on itthis!";
constexpr std::string_view RUNBOMB_GAME_MODE_DESC = "Running Bomb: \nDefault Minesweeper, but a random mine is a Running Bomb!\nThe Running Bomb move at each revealing click.\nIt moves only to an undiscovered tile, and can't move to a tile that already contains a mine.";

}

PlayMenu::PlayMenu()
	: gameModeText_{.origin = Text::Middle, .string = "Game Mode:"}
	, gameModeDescription_{.origin = Text::Top, .string = DEFAULT_GAME_MODE_DESC}
	, gameModeBtn_{.rect = {{}, BUTTON_SIZE}, .text = DEFAULT_GAME_MODE_TITLE}
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
		// TODO
	}
	else if (beginnerBtn_.rect.contains(position))
	{
		App::instance().game.setEasy();
		auto& layerStack = App::instance().layerStack;
		layerStack.scheduleAsyncCommand<LayerStack::Swap>(this, std::make_unique<BoardRenderer>());
		layerStack.scheduleAsyncCommand<LayerStack::Push>(std::make_unique<MinesweeperInput>());
		layerStack.scheduleAsyncCommand<LayerStack::Push>(std::make_unique<UserInterface>());
	}
	else if (intermediateBtn_.rect.contains(position))
	{
		App::instance().game.setMedium();
		auto& layerStack = App::instance().layerStack;
		layerStack.scheduleAsyncCommand<LayerStack::Swap>(this, std::make_unique<BoardRenderer>());
		layerStack.scheduleAsyncCommand<LayerStack::Push>(std::make_unique<MinesweeperInput>());
		layerStack.scheduleAsyncCommand<LayerStack::Push>(std::make_unique<UserInterface>());
	}
	else if (expertBtn_.rect.contains(position))
	{
		App::instance().game.setHard();
		auto& layerStack = App::instance().layerStack;
		layerStack.scheduleAsyncCommand<LayerStack::Swap>(this, std::make_unique<BoardRenderer>());
		layerStack.scheduleAsyncCommand<LayerStack::Push>(std::make_unique<MinesweeperInput>());
		layerStack.scheduleAsyncCommand<LayerStack::Push>(std::make_unique<UserInterface>());
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
