#pragma once
#include "Layer/Layer.h"

class MainMenu : public Layer
{
	EventConsumed handleEvent(const sf::Event& event) override;
	void render(sf::RenderTarget& target) const override;

private:


};
