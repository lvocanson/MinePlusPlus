#pragma once
#include "Layer.h"

class WindowInput : public Layer
{
	EventConsumed handleEvent(const sf::Event& event) override;
};

