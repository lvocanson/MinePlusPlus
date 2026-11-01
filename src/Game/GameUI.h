#pragma once
#include "Core/AppEvent.h"
#include "UI/Button.h"
#include "UI/Text.h"
#include "UI/ClickTracker.h"
#include <chrono>
#include <optional>
#include <string>

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
	UIEvent::Result operator()(const UIEvent::Typed&) { return UIEvent::Ignored; }
	void render(class UITarget& target) const;

private:

	void centerBoardOnView(App& app) const;
	std::optional<std::chrono::milliseconds> getBestTime() const;
	void formatGameInfo() const;
	void formatEndMessage() const;

private:

	Minesweeper& game_;
	std::optional<std::chrono::milliseconds> bestTime_;
	mutable std::string gameString_;
	mutable Text gameText_;
	Vec2i screenCenter_;
	Button restartBtn_, mainMenuBtn_, resetCameraBtn_;
	ClickTracker tracker_;
};
