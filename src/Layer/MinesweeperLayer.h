#pragma once
#include "Board.h"
#include "Layer.h"
#include <SFML/System/Clock.hpp>

class MinesweeperLayer : public Layer
{

public:

	MinesweeperLayer(Vec2s size, std::size_t mineCount);
	void centerBoardOnView();

private:

	EventConsumed listenEvent(const sf::Event& event) override;
	void render(sf::RenderTarget& target) const override;

public:

	EventConsumed operator()(const sf::Event::Closed& event);
	EventConsumed operator()(const sf::Event::Resized& event);
	EventConsumed operator()(const sf::Event::MouseMovedRaw& event);
	EventConsumed operator()(const sf::Event::MouseButtonPressed& event);
	EventConsumed operator()(const sf::Event::MouseButtonReleased& event);
	EventConsumed operator()(const sf::Event::MouseWheelScrolled& event);
	EventConsumed operator()(const sf::Event::KeyPressed& event);
	EventConsumed operator()(const auto& ignored) { return EventConsumed::No; }

private:

	Board board_;
	sf::Clock clock_;
	std::size_t pressedCellIdx_;
	bool moving_;
	bool gameEnd_;
};
