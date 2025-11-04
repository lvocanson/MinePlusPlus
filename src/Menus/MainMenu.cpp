#include "MainMenu.h"
#include "Core/App.h"
#include "PlayMenu.h"
#include "SettingsMenu.h"

namespace
{

constexpr sf::Vector2i BUTTON_SIZE = {150, 28};
constexpr sf::Vector2i ELEMENTS_GAP = {0, 60};
constexpr float EXIT_BUTTON_SCALE = 0.8f;
}

MainMenu::MainMenu()
	: title_{.origin = Text::Middle, .string = "Mine++!"}
	, playBtn_{.rect = {{}, BUTTON_SIZE}, .text = "Play"}
	, settingsBtn_{.rect = {{}, BUTTON_SIZE}, .text = "Settings"}
	, exitBtn_{.rect = {{}, sf::Vector2i(sf::Vector2f(BUTTON_SIZE) * EXIT_BUTTON_SCALE)}, .text = "Quit"}
{
}

void MainMenu::onPushed()
{
	UserInterfaceLayer::onPushed();
	App::instance().clearColor = {0x31, 0x4D, 0x79, 0x00};
}

void MainMenu::onScreenResized(sf::Vector2i newSize)
{
	sf::Vector2i center = newSize / 2;
	title_.position = center - ELEMENTS_GAP;
	playBtn_.rect.position = center - playBtn_.rect.size / 2;
	settingsBtn_.rect.position = center - settingsBtn_.rect.size / 2 + ELEMENTS_GAP;
	exitBtn_.rect.position = center - exitBtn_.rect.size / 2 + ELEMENTS_GAP * 2;
}

EventConsumed MainMenu::onMouseButtonReleased(sf::Vector2i position)
{
	if (playBtn_.rect.contains(position))
	{
		App::instance().layerStack.scheduleAsyncCommand<LayerStack::Swap>(this, std::make_unique<PlayMenu>());
	}
	else if (settingsBtn_.rect.contains(position))
	{
		App::instance().layerStack.scheduleAsyncCommand<LayerStack::Swap>(this, std::make_unique<SettingsMenu>());
	}
	else if (exitBtn_.rect.contains(position))
	{
		App::instance().exit();
	}
	else
		return EventConsumed::No;
	return EventConsumed::Yes;
}

void MainMenu::render(UITarget& target) const
{
	target.draw(title_);
	target.draw(playBtn_);
	target.draw(settingsBtn_);
	target.draw(exitBtn_);
}
