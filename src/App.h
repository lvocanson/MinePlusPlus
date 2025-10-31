#pragma once
#include "Board.h"
#include "Layer/LayerStack.h"
#include "Event/EventBus.h"
#include <SFML/Graphics/RenderWindow.hpp>
#include <concepts>
#include <vector>

class App
{
	static inline App* instance_{};

public:

	App();
	int run();
	void exit();
	~App();

	static App& instance() { return *instance_; }

public:

	sf::RenderWindow window;
	sf::Color clearColor;
	Board board;
	LayerStack layerStack;
	EventBus eventBus;
};
