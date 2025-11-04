#pragma once
#include "Layer/Layer.h"
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/RectangleShape.hpp>

class UserInterface : public Layer
{
public:

	UserInterface();

private:

	sf::Vector2f setupText(sf::Text& text) const;
	void setupBtn(sf::RectangleShape& rect, sf::Vector2f size) const;

	EventConsumed handleEvent(const sf::Event& event) override;
	void resizeUI(sf::Vector2f newSize);
	void centerBoardOnView();

	void update(float dt) override;
	void render(sf::RenderTarget& target) const override;

private:

	sf::View uiView_;
	sf::Text gameText_, restartText_, mainMenuText_, resetCameraText_;
	sf::RectangleShape restartBtn_, mainMenuBtn_, resetCameraBtn_;
};
