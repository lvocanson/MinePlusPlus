#include "App.h"
#include <chrono>
#include <iostream>
#include <SFML/Graphics/RenderTexture.hpp>

namespace
{

sf::Vector2u getDefaultWindowSize()
{
	sf::Vector2u screenSize = sf::VideoMode::getDesktopMode().size;
	return sf::Vector2u(sf::Vector2f(screenSize) * 0.75f);
}

sf::ContextSettings getDefaultContextSettings()
{
	return sf::ContextSettings
	{
		.antiAliasingLevel = sf::RenderTexture::getMaximumAntiAliasingLevel()
	};
}

} // namespace

App::App()
	: window(sf::VideoMode(getDefaultWindowSize()), "Mine++", sf::State::Windowed, getDefaultContextSettings())
	, clearColor(sf::Color::Magenta)
{
	if (instance_)
		throw;
	instance_ = this;

	window.setVerticalSyncEnabled(true);
}

int App::run()
{
	auto lastFrame = std::chrono::steady_clock::now();
	while (window.isOpen())
	{
		layerStack.processAsyncCommands();

		while (auto event = window.pollEvent())
			layerStack.handleEvent(*event);

		auto now = std::chrono::steady_clock::now();
		float dt = std::chrono::duration_cast<std::chrono::nanoseconds>(now - lastFrame).count() / 1e9f;
		lastFrame = now;

		layerStack.update(dt);
		window.clear(clearColor);
		layerStack.render(window);
		window.display();
	}

	return EXIT_SUCCESS;
}

void App::exit()
{
	window.close();
}

App::~App()
{
	// Delete layers before calling destructors of other members
	layerStack.scheduleAsyncCommand<LayerStack::Clear>();
	layerStack.processAsyncCommands();
}
