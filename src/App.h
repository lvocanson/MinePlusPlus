#pragma once
#include "IScene.h"
#include <SFML/Graphics/RenderWindow.hpp>

class App
{
public:

	App();
	int run();

private:

	void onSceneChanged();

public:
	
	void operator()(const sf::Event::Closed&);
	void operator()(const sf::Event::Resized& data);
	void operator()(const sf::Event::MouseButtonPressed& data);
	void operator()(const sf::Event::MouseButtonReleased& data);
	void operator()(const sf::Event::KeyPressed& data);
	void operator()(auto&) {} // ignore other events

private:

	sf::RenderWindow window_;
	std::unique_ptr<IScene> scene_;
};
