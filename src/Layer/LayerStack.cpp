#include "LayerStack.h"

void LayerStack::handleEvent(const sf::Event& event)
{
	std::size_t size = layerStack_.size();
	for (std::size_t i = size - 1; i < size; --i)
	{
		auto& layer = layerStack_[i];
		auto consumed = layer->handleEvent(event);
		if (consumed == EventConsumed::Yes)
			break;
	}
}

void LayerStack::update(float dt)
{
	for (auto& layer : layerStack_)
		if (layer)
			layer->update(dt);
}

void LayerStack::render(sf::RenderTarget& target) const
{
	for (auto& layer : layerStack_)
		if (layer)
			layer->render(target);
}

void LayerStack::swap(Layer* layer, std::unique_ptr<Layer>&& substitute)
{
	std::size_t size = layerStack_.size();
	for (std::size_t i = 0; i < size; ++i)
		if (layerStack_[i].get() == layer)
		{
			layerStack_[i] = std::move(substitute);
			return;
		}
}

void LayerStack::remove(Layer* layer)
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
