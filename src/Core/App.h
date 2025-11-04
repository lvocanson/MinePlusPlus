#pragma once
#include "Game/Minesweeper.h"
#include "Layer/LayerStack.h"
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
	Minesweeper game;
	LayerStack layerStack;
};
