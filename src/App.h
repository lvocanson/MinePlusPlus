#pragma once
#include "GameScene.h"
#include <variant>

class App
{
public:

	App();
	int run(); 

private:

	void sceneExec(auto&& func) { std::visit(std::forward<decltype(func)>(func), scene_); };

public:

	void operator()(const sf::Event::Closed&);
	void operator()(const sf::Event::Resized& data);
	void operator()(const sf::Event::MouseButtonPressed& data);
	void operator()(const sf::Event::MouseButtonReleased& data);
	void operator()(const sf::Event::KeyPressed& data);
	void operator()(auto&) {} // ignore other events

private:

	sf::RenderWindow window_;
	std::variant<GameScene> scene_;
};
