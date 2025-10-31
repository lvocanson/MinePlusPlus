#pragma once
#include "Layer.h"
#include <concepts>
#include <memory>
#include <vector>

class LayerStack
{
public:

	void handleEvent(const sf::Event& event);
	void update(float dt);
	void render(sf::RenderTarget& target) const;

public:

	void clear() { layerStack_.clear(); }
	void push(std::unique_ptr<Layer>&& layer) { layerStack_.emplace_back(std::move(layer)); }
	void swap(Layer* layer, std::unique_ptr<Layer>&& substitute);
	void remove(Layer* layer);

private:

	std::vector<std::unique_ptr<Layer>> layerStack_;
};

