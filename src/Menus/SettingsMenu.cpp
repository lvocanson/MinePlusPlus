#include "SettingsMenu.h"
#include "MainMenu.h"
#include "Core/App.h"

UIEvent::Result SettingsMenu::operator()(const UIEvent::Resized& event)
{
	// TODO: Real settings menu
	event.app.submitCommand<SwapUI>(SwapUI::DEFAULT<MainMenu>);
	return UIEvent::Consumed;
}

UIEvent::Result SettingsMenu::operator()(const UIEvent::Pressed& event)
{
	return UIEvent::Consumed;
}

UIEvent::Result SettingsMenu::operator()(const UIEvent::Released& event)
{
	return UIEvent::Consumed;
}

void SettingsMenu::render(UITarget& target) const {}
