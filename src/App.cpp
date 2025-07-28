#include "App.h"
#include "MenuScene.h"

App::App()
	: window_(sf::VideoMode({900, 600}), "Mine++")
	, scene_(std::make_unique<MenuScene>())
{
	window_.setVerticalSyncEnabled(true);
	onSceneChanged();
}

int App::run()
{
	assert(scene_);

	while (window_.isOpen())
	{
		while (auto event = window_.pollEvent())
			event->visit(*this);

		if (auto ptr = scene_->update())
		{
			scene_ = std::move(ptr);
			onSceneChanged();
			continue;
		}

		scene_->drawOn(window_);
		window_.display();
	}

	return EXIT_SUCCESS;
}

void App::onSceneChanged()
{
	scene_->onWindowResize(window_.getSize());
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
	scene_->onWindowResize(data.size);
}

void App::operator()(const sf::Event::MouseButtonPressed& data)
{
	auto coords = window_.mapPixelToCoords(data.position, window_.getView());
	scene_->onMouseButtonPressed(coords, data.button);
}

void App::operator()(const sf::Event::MouseButtonReleased& data)
{
	auto coords = window_.mapPixelToCoords(data.position, window_.getView());
	scene_->onMouseButtonReleased(coords, data.button);
}

void App::operator()(const sf::Event::KeyPressed& data)
{
	scene_->onKeyPressed(data.code);
}
