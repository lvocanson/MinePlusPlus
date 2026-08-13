#pragma once
#include "Core/AppEvent.h"
#include "UI/Button.h"
#include "UI/Text.h"
#include "UI/ClickTracker.h"

class PlayMenu
{
public:

	PlayMenu(class Minesweeper&);
	UIEvent::Result operator()(const UIEvent::Resized& event);
	UIEvent::Result operator()(const UIEvent::Pressed& event);
	UIEvent::Result operator()(const UIEvent::Released& event);
	UIEvent::Result operator()(const UIEvent::Typed& event) { return UIEvent::Ignored; }
	void render(class UITarget& target) const;

	enum GameMode
	{
		Default,
		Spinning,
		RunningBomb,
		Count
	};

private:

	enum Difficulty { Easy, Medium, Hard };

	void play(App&, Difficulty) const;

	GameMode gameMode_;
	Text gameModeText_, gameModeDescription_;
	Button gameModeBtn_, beginnerBtn_, intermediateBtn_, expertBtn_, backBtn_, customBtn_;
	ClickTracker tracker_;
};
