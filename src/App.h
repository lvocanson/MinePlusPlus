#pragma once
#include "Layer/Layer.h"
#include <SFML/Graphics/RenderWindow.hpp>
#include <concepts>
#include <vector>

class App
{
public:

	App();
	int run();
	void exit();
	static App& instance() { return *instance_; }

public: // Window

	sf::Vector2u getWindowSize() const { return window_.getSize(); }

	const sf::View& getView() const { return window_.getView(); }
	void setView(const sf::View& view) { return window_.setView(view); }

	void setClearColor(sf::Color color) { clearColor_ = color; }
	sf::Vector2f screenToWorld(sf::Vector2i position);

public: // Layers

	template <std::derived_from<Layer> T, class... Args>
	T* pushLayer(Args&&... args)
	{
		std::unique_ptr unique = std::make_unique<T>(std::forward<Args>(args)...);
		T* ptr = unique.get();
		layerStack_.emplace_back(std::move(unique));
		return ptr;
	}

	void removeLayer(Layer* layer);

	template <std::derived_from<Layer> T, class... Args>
	T* swapLayer(Layer* toRemove, Args&&... args)
	{
		for (auto& layer : layerStack_)
			if (layer.get() == toRemove)
			{
				std::unique_ptr unique = std::make_unique<T>(std::forward<Args>(args)...);
				T* ptr = unique.get();
				layer = std::move(unique);
				return ptr;
			}

		return nullptr;
	}

private:

	static inline App* instance_{};
	sf::RenderWindow window_;
	sf::Color clearColor_;
	std::vector<std::unique_ptr<Layer>> layerStack_;
};
