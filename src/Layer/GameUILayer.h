#pragma once
#include "BaseUILayer.h"
#include "Resources.h"
#include <SFML/System/Clock.hpp>

class GameUILayer : public BaseUILayer
{
	static constexpr auto& FONT_REF = Resources::Fonts::roboto;
	static constexpr unsigned FONT_SIZE = 20;

public:

	GameUILayer();

private:

	EventConsumed onPress(sf::Mouse::Button button, sf::Vector2f position) override;
	EventConsumed onRelease(sf::Mouse::Button button, sf::Vector2f position) override;

	void restart();
	void centerBoardOnView();

	void render(sf::RenderTarget& target) const override;

private:

	sf::Clock clock_;
	class Board* board_;
	std::size_t pressedCellIdx_;
};
