#pragma once
#include "Core/AppEvent.h"
#include "UI/Button.h"
#include "UI/Text.h"
#include "UI/ClickTracker.h"
#include <SFML/System/String.hpp>

class App;
class Minesweeper;

class GameUI
{
public:

	GameUI(App& app);
	~GameUI();

	UIEvent::Result operator()(const UIEvent::Resized& event);
	UIEvent::Result operator()(const UIEvent::Pressed& event);
	UIEvent::Result operator()(const UIEvent::Released& event);
	UIEvent::Result operator()(const UIEvent::Typed& event) { return UIEvent::Ignored; }
	void render(class UITarget& target) const;

private:

	void centerBoardOnView(App& app) const;

private:

	Minesweeper& game_;
	mutable sf::String gameString_;
	mutable Text gameText_;
	Button restartBtn_, mainMenuBtn_, resetCameraBtn_;
	ClickTracker tracker_;
};
