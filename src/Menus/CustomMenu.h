#pragma once
#include "Core/AppEvent.h"
#include "UI/Button.h"
#include "UI/NumberField.h"
#include "UI/Text.h"
#include "UI/ClickTracker.h"
#include "UI/Cursor.h"

class Minesweeper;

class CustomMenu
{
public:

	CustomMenu(Minesweeper&);
	UIEvent::Result operator()(const UIEvent::Resized& event);
	UIEvent::Result operator()(const UIEvent::Pressed& event);
	UIEvent::Result operator()(const UIEvent::Released& event);
	UIEvent::Result operator()(const UIEvent::Typed& event);
	void render(class UITarget& target) const;

private:

	Minesweeper& game_;
	sf::String diagnosticStr_;
	Text widthText_, heightText_, minesText_, diagnosticText_;
	NumberField widthField_, heightField_, minesField_;
	Button backBtn_, startBtn_;
	Cursor cursor_;
	ClickTracker tracker_;
};

