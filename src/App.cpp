#include "App.h"
#include "GameScene.h"

App::App()
	: window_(sf::VideoMode({900, 600}), "Mine++")
	, scene_(std::in_place_type_t<GameScene>{}, Vec2s{16, 16}, 40)
{
	window_.setVerticalSyncEnabled(true);
	sceneExec([&](Scene auto& scene) { scene.onWindowResize(window_.getSize()); });
}

int App::run()
{

	while (window_.isOpen())
	{
		while (auto event = window_.pollEvent())
			event->visit(*this);

		sceneExec([&](Scene auto& scene) { scene.update(); });
		sceneExec([&](Scene auto& scene) { scene.drawOn(window_); });
		window_.display();
	}

	return EXIT_SUCCESS;
}

void App::operator()(const sf::Event::Closed&) 
{
	window_.close();
}

void App::operator()(const sf::Event::Resized& data)
{
	sf::View view = window_.getView();
	sf::Vector2f size(data.size);
	view.setSize(size);
	view.setCenter(size / 2.f);
	window_.setView(view);
	sceneExec([&](Scene auto& scene) { scene.onWindowResize(data.size); });
}

void App::operator()(const sf::Event::MouseButtonPressed& data)
{
	auto coords = window_.mapPixelToCoords(data.position, window_.getView());
	sceneExec([&](Scene auto& scene) { scene.onMouseButtonPressed(coords, data.button); });
}

void App::operator()(const sf::Event::MouseButtonReleased& data)
{
	auto coords = window_.mapPixelToCoords(data.position, window_.getView());
	sceneExec([&](Scene auto& scene) { scene.onMouseButtonReleased(coords, data.button); });
}

void App::operator()(const sf::Event::KeyPressed& data)
{
	sceneExec([&](Scene auto& scene) { scene.onKeyPressed(data.code); });
}
