#pragma once
#include "Layer.h"
#include "Event/MinesweeperEvents.h"

/**
 * Renders the minesweeper board.
 */
class BoardRenderer : public Layer
{

public:

	BoardRenderer();
	~BoardRenderer();

private:

	void render(sf::RenderTarget& target) const override;

private:

	std::size_t selectedCell_;
	MinesweeperEvents::RenderMode renderMode_;
};
