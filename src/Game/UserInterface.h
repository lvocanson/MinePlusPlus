#pragma once
#include "UI/UserInterfaceLayer.h"
#include "UI/Button.h"
#include "UI/Text.h"

class UserInterface : public UserInterfaceLayer
{
public:

	UserInterface();

private:

	EventConsumed handleEvent(const sf::Event& event) override;
	void centerBoardOnView();
	void update(float dt) override;

	void onScreenResized(sf::Vector2i newSize) override;
	void render(UITarget& target) const override;

private:

	Text gameText_;
	Button restartBtn_, mainMenuBtn_, resetCameraBtn_;
};
