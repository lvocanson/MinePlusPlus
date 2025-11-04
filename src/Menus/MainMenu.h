#pragma once
#include "UI/UserInterfaceLayer.h"
#include "UI/Button.h"
#include "UI/Text.h"

class MainMenu : public UserInterfaceLayer
{
public:

	MainMenu();

private:

	void onPushed() override;
	void onScreenResized(sf::Vector2i newSize) override;
	EventConsumed onMouseButtonReleased(sf::Vector2i position) override;
	void render(UITarget& target) const override;

private:

	Text title_;
	Button playBtn_, settingsBtn_, exitBtn_;
};
