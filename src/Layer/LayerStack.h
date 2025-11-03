#pragma once
#include "Layer.h"
#include <concepts>
#include <memory>
#include <variant>
#include <vector>

class LayerStack
{
public:

	template <class T, class... Args>
	void scheduleAsyncCommand(Args&&... args)
	{
		commandQueue_.emplace_back(std::in_place_type_t<T>{}, std::forward<Args>(args)...);
	}
	void processAsyncCommands();

	void handleEvent(const sf::Event& event);
	void update(float dt);
	void render(sf::RenderTarget& target) const;

public: // Async commands

	struct Clear {};
	struct Push
	{
		std::unique_ptr<Layer> layer;
	};
	struct Swap
	{
		Layer* layer;
		std::unique_ptr<Layer> substitute;
	};
	struct Remove
	{
		Layer* layer;
	};
	using Command = std::variant<Clear, Push, Swap, Remove>;

private:

	std::vector<std::unique_ptr<Layer>> layerStack_;
	std::vector<Command> commandQueue_;
};

