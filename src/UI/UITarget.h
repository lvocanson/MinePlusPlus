#pragma once
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/View.hpp>

class UITarget
{
	friend class App;
	UITarget(sf::RenderTarget& target);
	~UITarget();

	sf::RenderTarget& target_;
	sf::View view_;
	static sf::RectangleShape rect_;
	static sf::Text text_;

public:

	void draw(const struct Button& button);
	void draw(const struct Text& text);
	void draw(const struct NumberField& field);
	void draw(const struct Cursor& cursor);
};
