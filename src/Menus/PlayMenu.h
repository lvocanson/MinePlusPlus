#pragma once
#include "UI/UserInterfaceLayer.h"
#include "UI/Button.h"
#include "UI/Text.h"

class PlayMenu : public UserInterfaceLayer
{
public:

	PlayMenu();

private:

	void onScreenResized(sf::Vector2i newSize) override;
	EventConsumed onMouseButtonReleased(sf::Vector2i position) override;
	void play();
	void render(UITarget& target) const override;

private:

	unsigned gameMode_;
	Text gameModeText_, gameModeDescription_;
	Button gameModeBtn_, beginnerBtn_, intermediateBtn_, expertBtn_, backBtn_, customBtn_;
};
