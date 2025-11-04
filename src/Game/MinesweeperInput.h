#pragma once
#include "Layer/Layer.h"

class MinesweeperInput : public Layer
{
	EventConsumed handleEvent(const sf::Event& event) override;

	bool isMouseDraggingCamera_ = false;
};
