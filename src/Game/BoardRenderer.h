#pragma once
#include "Layer/Layer.h"

/**
 * Renders the minesweeper board.
 */
class BoardRenderer : public Layer
{

public:

	BoardRenderer();

private:

	EventConsumed handleEvent(const sf::Event& event) override;
	void render(sf::RenderTarget& target) const override;

private:

	bool cheat_;
};
