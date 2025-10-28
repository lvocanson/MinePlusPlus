#include "App.h"
#include <chrono>

App::App()
	: window_(sf::VideoMode({900, 600}), "Mine++")
	, clearColor_(sf::Color::Magenta)
{
	if (instance_)
		throw;
	instance_ = this;

	window_.setVerticalSyncEnabled(true);
}

int App::run()
{
	auto lastFrame = std::chrono::steady_clock::now();
	while (window_.isOpen())
	{
		while (auto event = window_.pollEvent())
			for (auto it = layerStack_.rbegin(); it != layerStack_.rend(); ++it)
				if (*it)
				{
					auto consumed = (*it)->listenEvent(*event);
					if (consumed == EventConsumed::Yes)
						break;
				}

		auto now = std::chrono::steady_clock::now();
		float dt = std::chrono::duration_cast<std::chrono::nanoseconds>(now - lastFrame).count() / 1e6f;
		lastFrame = now;

		for (auto& layer : layerStack_)
			if (layer)
				layer->update(dt);

		window_.clear(clearColor_);

		for (auto& layer : layerStack_)
			if (layer)
				layer->render(window_);

		window_.display();
	}

	return EXIT_SUCCESS;
}

void App::exit()
{
	window_.close();
}

sf::Vector2f App::screenToWorld(sf::Vector2i position)
{
	return window_.mapPixelToCoords(position, window_.getView());
}

void App::removeLayer(Layer* layer)
{
	auto it = layerStack_.begin();
	while (it != layerStack_.end())
	{
		if (it->get() == layer)
		{
			it = layerStack_.erase(it);
			return;
		}
		++it;
	}
}
