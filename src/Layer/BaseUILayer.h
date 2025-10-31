#pragma once
#include "Layer.h"
#include <SFML/Graphics.hpp>

/**
 * Common UI needs.
 */
class BaseUILayer : public Layer
{
	struct EventDispatcher;

protected:

	bool isMouseDraggingCamera_ = false;
	bool isMouseWheelControllingZoom_ = false;

	EventConsumed handleEvent(const sf::Event& event) override;

	virtual EventConsumed onPress(sf::Mouse::Button button, sf::Vector2f position) { return EventConsumed::No; }
	virtual EventConsumed onRelease(sf::Mouse::Button button, sf::Vector2f position) { return EventConsumed::No; }

};

