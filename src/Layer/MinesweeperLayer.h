#pragma once
#include "Layer.h"
#include "Board.h"
#include "Event/MinesweeperEvents.h"

/**
 * Encapsulate a board and renders it.
 */
class MinesweeperLayer : public Layer
{

public:

	MinesweeperLayer();
	~MinesweeperLayer();

private:

	void render(sf::RenderTarget& target) const override;

	void restart();

private:

	Board board_;
	std::size_t selectedCell_;
	MinesweeperEvents::RenderMode renderMode_;
};
