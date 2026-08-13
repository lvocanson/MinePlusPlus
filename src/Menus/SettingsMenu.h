#pragma once
#include "Core/AppEvent.h"
#include "UI/Button.h"
#include "UI/Text.h"
#include "UI/ClickTracker.h"

class SettingsMenu
{
public:

	UIEvent::Result operator()(const UIEvent::Resized& event);
	UIEvent::Result operator()(const UIEvent::Pressed& event);
	UIEvent::Result operator()(const UIEvent::Released& event);
	UIEvent::Result operator()(const UIEvent::Typed& event) { return UIEvent::Ignored; }
	void render(class UITarget& target) const;
};
