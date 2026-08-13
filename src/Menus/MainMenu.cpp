#include "MainMenu.h"
#include "Core/App.h"
#include "PlayMenu.h"
#include "SettingsMenu.h"
#include "UI/UITarget.h"

namespace
{

constexpr sf::Vector2i BUTTON_SIZE = {150, 28};
constexpr sf::Vector2i ELEMENTS_GAP = {0, 60};
constexpr float EXIT_BUTTON_SCALE = 0.8f;

}

MainMenu::MainMenu()
	: title_{.origin = Text::Middle, .string = U"Mine++!"}
	, playBtn_{.rect = {{}, BUTTON_SIZE}, .text = U"Play"}
	, settingsBtn_{.rect = {{}, BUTTON_SIZE}, .text = U"Settings"}
	, exitBtn_{.rect = {{}, sf::Vector2i(sf::Vector2f(BUTTON_SIZE) * EXIT_BUTTON_SCALE)}, .text = U"Quit"} {}

UIEvent::Result MainMenu::operator()(const UIEvent::Resized& event)
{
	sf::Vector2i centerView = event.newSize / 2;
	title_.position = centerView - ELEMENTS_GAP;
	playBtn_.rect.position = centerView - playBtn_.rect.size / 2;
	settingsBtn_.rect.position = centerView - settingsBtn_.rect.size / 2 + ELEMENTS_GAP;
	exitBtn_.rect.position = centerView - exitBtn_.rect.size / 2 + ELEMENTS_GAP * 2;
	return UIEvent::Consumed;
}

UIEvent::Result MainMenu::operator()(const UIEvent::Pressed& event)
{
	return tracker_.registerPress(
		       event.position,
		       {
			       playBtn_,
			       settingsBtn_,
			       exitBtn_
		       })
	       ? UIEvent::Consumed
	       : UIEvent::Ignored;
}

UIEvent::Result MainMenu::operator()(const UIEvent::Released& event)
{
	if (tracker_.isClicked(playBtn_, event.position))
	{
		event.app.submitCommand<SwapUI>([&game = event.app.getGame()](AppUI& ui) { ui.emplace<PlayMenu>(game); });
	}
	else if (tracker_.isClicked(settingsBtn_, event.position))
	{
		event.app.submitCommand<SwapUI>(SwapUI::DEFAULT<SettingsMenu>);
	}
	else if (tracker_.isClicked(exitBtn_, event.position))
	{
		event.app.submitCommand<RequestExit>();
	}
	else
		return UIEvent::Ignored;
	return UIEvent::Consumed;
}

void MainMenu::render(UITarget& target) const
{
	target.draw(title_);
	target.draw(playBtn_);
	target.draw(settingsBtn_);
	target.draw(exitBtn_);
}
