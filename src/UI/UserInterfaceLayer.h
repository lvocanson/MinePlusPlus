#pragma once
#include "Layer/Layer.h"
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Text.hpp>

/*
 * Replace the Layer::render method with one specialised for UI.
 */
class UserInterfaceLayer : public Layer
{
private:

	void render(sf::RenderTarget& target) const final;

protected:

	class UITarget
	{
		friend UserInterfaceLayer;
		UITarget(sf::RenderTarget& target);
		sf::RenderTarget& target_;

		sf::RectangleShape rect_;
		sf::Text text_;

	public:

		void draw(const struct Button& button);
		void draw(const struct Text& text);
	};

	void onPushed() override;
	EventConsumed handleEvent(const sf::Event& event) override;

	virtual void onScreenResized(sf::Vector2i newSize) {}
	virtual EventConsumed onMouseButtonReleased(sf::Vector2i position) { return EventConsumed::No; }
	virtual void render(UITarget& target) const {}

private:

	sf::View uiView_;
};
