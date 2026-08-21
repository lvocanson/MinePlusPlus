#pragma once
#include "Text.h"
#include <SFML/Graphics/Glyph.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Vertex.hpp>
#include <SFML/Graphics/View.hpp>
#include <array>

/*
 * Draws the UI in screen space.
 * Text rendering does not allocate.
 */
class UITarget
{
	friend class App;
	UITarget(sf::RenderTarget& target);
	~UITarget();

public:

	void draw(const struct Button& button);
	void draw(const Text& text);
	void draw(const struct NumberField& field);
	void draw(const struct Cursor& cursor);

private:

	static constexpr std::size_t VERTICES_PER_GLYPH = 6;
	static constexpr std::size_t GLYPH_CAPACITY = 32; // Trade memory for draw calls
	static constexpr std::size_t VERTEX_CAPACITY = GLYPH_CAPACITY * VERTICES_PER_GLYPH;

	void addGlyph(const sf::Glyph& fillGlyph, const sf::Glyph& outlineGlyph, sf::Vector2f pen, sf::Color color);
	void flushText();

	template <class Sequence>
	void pushText(Sequence&& sequence, sf::Vector2f position, Text::Origin origin, sf::Color color);

	sf::RenderTarget& target_;
	sf::View view_;
	std::array<sf::Vertex, VERTEX_CAPACITY> fillBatch_, outlineBatch_;
	std::size_t batchCount_;
	static sf::RectangleShape rect_;
};
