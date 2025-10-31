#pragma once
#include "Layer.h"

class MenuLayer : public Layer
{
	EventConsumed handleEvent(const sf::Event& event) override;
	void render(sf::RenderTarget& target) const override;

private:


};
