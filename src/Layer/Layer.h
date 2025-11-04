#pragma once
#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

enum class EventConsumed
{
	No = 0,
	Yes,
};

class Layer
{
public:

	virtual ~Layer() = default;
	virtual void onPushed() {};

	virtual EventConsumed handleEvent(const sf::Event& event) { return EventConsumed::No; }
	virtual void update(float dt) {}
	virtual void render(sf::RenderTarget& target) const {}
};
