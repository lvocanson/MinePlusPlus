#include "LayerStack.h"
#include "Utils/Overloaded.h"

namespace
{
template <class T> struct MissingCommand : std::false_type {};
}

void LayerStack::processAsyncCommands()
{
	for (auto& command : commandQueue_) std::visit(Overloaded
		{
			[this](Clear& cmd)
			{
				layerStack_.clear();
			},
			[this](Push& cmd)
			{
				auto& layer = layerStack_.emplace_back(std::move(cmd.layer));
				layer->onPushed();
			},
			[this](Swap& cmd)
			{
				for (auto& layer : layerStack_)
					if (layer.get() == cmd.layer)
					{
						layer = std::move(cmd.substitute);
						layer->onPushed();
						return;
					}
			},
			[this](Remove& cmd)
			{
				auto it = layerStack_.begin();
				while (it != layerStack_.end())
				{
					if (it->get() == cmd.layer)
					{
						it = layerStack_.erase(it);
						return;
					}
					++it;
				}
			},
			[](const auto&)
			{
				static_assert(false, "Missing command implementation.");
			}
		}, command);
	commandQueue_.clear();
}

void LayerStack::handleEvent(const sf::Event& event)
{
	for (std::size_t i = layerStack_.size(); i-- > 0;)
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
