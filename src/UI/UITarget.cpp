#include "UITarget.h"
#include "Game/Resources.h"
#include "Button.h"
#include "NumberField.h"
#include "Cursor.h"
#include <SFML/System/Utf.hpp>
#include <algorithm>

namespace
{

constexpr auto& FONT_REF = Resources::Fonts::roboto;
constexpr unsigned FONT_SIZE = 20;
constexpr float OUTLINE_THICKNESS = 1.f;
constexpr sf::Color FILL_COLOR = sf::Color::White;
constexpr sf::Color OUTLINE_COLOR = sf::Color::Black;
constexpr int CURSOR_WIDTH = 2;
constexpr int CURSOR_DIST_FROM_TEXT = 2;
constexpr float RECT_OUTLINE_THICKNESS = 2.f;

// Writes the two triangles of a glyph quad. 'out' must have room for 6 vertices.
void writeGlyphQuad(sf::Vertex* out, const sf::Glyph& glyph, sf::Vector2f pen, sf::Color color)
{
	sf::Vector2f topLeft = pen + glyph.bounds.position;
	sf::Vector2f botRight = topLeft + glyph.bounds.size;
	sf::Vector2f uvTopLeft(glyph.textureRect.position);
	sf::Vector2f uvBotRight = uvTopLeft + sf::Vector2f(glyph.textureRect.size);

	out[0] = {{topLeft.x, topLeft.y}, color, {uvTopLeft.x, uvTopLeft.y}};
	out[1] = {{botRight.x, topLeft.y}, color, {uvBotRight.x, uvTopLeft.y}};
	out[2] = {{topLeft.x, botRight.y}, color, {uvTopLeft.x, uvBotRight.y}};
	out[3] = out[1];
	out[4] = {{botRight.x, botRight.y}, color, {uvBotRight.x, uvBotRight.y}};
	out[5] = out[2];
}

struct GlyphLayout
{
	sf::Vector2f pen{};
	char32_t previous{};
	sf::Vector2f inkMin{}, inkMax{};
	bool hasInk{};

	// Calls sink(codePoint, glyph, pen) for each code point that has a quad.
	template <class Sink>
	void put(char32_t codePoint, Sink&& sink)
	{
		if (codePoint == U'\r')
			return;

		pen.x += FONT_REF.getKerning(previous, codePoint, FONT_SIZE);
		previous = codePoint;

		switch (codePoint)
		{
		case U'\n': pen = {0.f, pen.y + FONT_REF.getLineSpacing(FONT_SIZE)};
			return;
		case U'\t': pen.x += FONT_REF.getGlyph(U' ', FONT_SIZE, false).advance * 4.f;
			return;
		default: break;
		}

		const sf::Glyph& glyph = FONT_REF.getGlyph(codePoint, FONT_SIZE, false);

		// Whitespace has an advance but nothing to rasterize
		if (glyph.bounds.size.x != 0.f && glyph.bounds.size.y != 0.f)
		{
			sf::Vector2f min = pen + glyph.bounds.position;
			sf::Vector2f max = min + glyph.bounds.size;
			inkMin = hasInk ? sf::Vector2f{std::min(inkMin.x, min.x), std::min(inkMin.y, min.y)} : min;
			inkMax = hasInk ? sf::Vector2f{std::max(inkMax.x, max.x), std::max(inkMax.y, max.y)} : max;
			hasInk = true;
			sink(codePoint, glyph, pen);
		}

		pen.x += glyph.advance;
	}

	sf::FloatRect bounds() const
	{
		if (!hasInk)
			return {};

		sf::Vector2f outline(OUTLINE_THICKNESS, OUTLINE_THICKNESS);
		return {inkMin - outline, (inkMax - inkMin) + outline * 2.f};
	}
};

sf::Vector2f anchorOf(const sf::FloatRect& bounds, Text::Origin origin)
{
	switch (origin)
	{
	case Text::TopLeft: return bounds.position;
	case Text::Top: return {bounds.position.x + bounds.size.x / 2.f, bounds.position.y};
	case Text::TopRight: return bounds.position + sf::Vector2f(bounds.size.x, 0.f);
	case Text::Left: return {bounds.position.x, bounds.position.y + bounds.size.y / 2.f};
	case Text::Middle: return bounds.getCenter();
	case Text::Right: return {bounds.position.x + bounds.size.x, bounds.position.y + bounds.size.y / 2.f};
	case Text::BotLeft: return bounds.position + sf::Vector2f(0.f, bounds.size.y);
	case Text::Bot: return {bounds.position.x + bounds.size.x / 2.f, bounds.position.y + bounds.size.y};
	case Text::BotRight: return bounds.position + bounds.size;
	default: return {};
	}
}

// Decodes UTF-8 on the fly, without any intermediate buffer.
auto codePointsOf(std::string_view utf8)
{
	return [utf8](auto&& put)
	{
		for (auto it = utf8.begin(); it != utf8.end();)
		{
			char32_t codePoint{};
			it = sf::Utf8::decode(it, utf8.end(), codePoint);
			put(codePoint);
		}
	};
}

// Emits the decimal digits, most significant first, by successive divisions.
auto codePointsOf(std::size_t value)
{
	return [value](auto&& put)
	{
		std::size_t divisor = 1;
		for (std::size_t rest = value; rest >= 10; rest /= 10)
			divisor *= 10;

		do
		{
			put(char32_t(U'0' + (value / divisor) % 10));
			divisor /= 10;
		} while (divisor != 0);
	};
}

template <class Sequence>
sf::FloatRect textSize(Sequence&& sequence)
{
	GlyphLayout layout;
	sequence(
		[&](char32_t codePoint)
		{
			layout.put(codePoint, [](char32_t, const sf::Glyph&, sf::Vector2f) {});
		});
	return layout.bounds();
}

} // namespace

sf::RectangleShape UITarget::rect_{};

UITarget::UITarget(sf::RenderTarget& target)
	: target_(target)
	, view_(target.getView())
	, batchCount_(0)
{
	sf::Vector2f size(target.getSize());
	target.setView({size / 2.f, size});

	rect_.setOutlineThickness(RECT_OUTLINE_THICKNESS);
	rect_.setOutlineColor(sf::Color::White);
}

UITarget::~UITarget()
{
	flushText();
	target_.setView(view_);
}

void UITarget::addGlyph(const sf::Glyph& fillGlyph, const sf::Glyph& outlineGlyph, sf::Vector2f pen)
{
	if (batchCount_ + VERTICES_PER_GLYPH > VERTEX_CAPACITY)
		flushText();

	writeGlyphQuad(&outlineBatch_[batchCount_], outlineGlyph, pen, OUTLINE_COLOR);
	writeGlyphQuad(&fillBatch_[batchCount_], fillGlyph, pen, FILL_COLOR);
	batchCount_ += VERTICES_PER_GLYPH;
}

void UITarget::flushText()
{
	if (batchCount_ == 0)
		return;

	sf::RenderStates states;
	states.texture = &FONT_REF.getTexture(FONT_SIZE);
	states.coordinateType = sf::CoordinateType::Pixels;

	target_.draw(outlineBatch_.data(), batchCount_, sf::PrimitiveType::Triangles, states);
	target_.draw(fillBatch_.data(), batchCount_, sf::PrimitiveType::Triangles, states);
	batchCount_ = 0;
}

template <class Sequence>
void UITarget::pushText(Sequence&& sequence, sf::Vector2f position, Text::Origin origin)
{
	sf::Vector2f offset = position - anchorOf(textSize(sequence), origin);
	GlyphLayout layout;
	sequence(
		[&](char32_t codePoint)
		{
			layout.put(
				codePoint,
				[&](char32_t cp, const sf::Glyph& glyph, sf::Vector2f pen)
				{
					addGlyph(glyph, FONT_REF.getGlyph(cp, FONT_SIZE, false, OUTLINE_THICKNESS), pen + offset);
				});
		});
}

void UITarget::draw(const Button& button)
{
	sf::FloatRect rect(button.rect);
	rect_.setPosition(rect.position);
	rect_.setSize(rect.size);
	rect_.setFillColor({0x79, 0xB6, 0x1E, 0xFF});
	target_.draw(rect_);

	pushText(codePointsOf(button.text), rect.getCenter(), Text::Middle);
}

void UITarget::draw(const Text& text)
{
	pushText(codePointsOf(text.string), sf::Vector2f(text.position), text.origin);
}

void UITarget::draw(const NumberField& field)
{
	sf::FloatRect rect(field.rect);
	rect_.setPosition(rect.position);
	rect_.setSize(rect.size);
	rect_.setFillColor(sf::Color::White);
	target_.draw(rect_);

	pushText(codePointsOf(field.value), rect.getCenter(), Text::Middle);
}

void UITarget::draw(const Cursor& cursor)
{
	if (!cursor.field)
		return;

	sf::FloatRect fieldRect(cursor.field->rect);
	sf::FloatRect bounds = textSize(codePointsOf(cursor.field->value));

	sf::Vector2f size = {CURSOR_WIDTH, FONT_REF.getLineSpacing(FONT_SIZE) * 0.9f};
	rect_.setSize(size);
	rect_.setFillColor(sf::Color::Black);
	rect_.setOutlineThickness(0.f);
	rect_.setPosition(
		fieldRect.getCenter() + sf::Vector2f(bounds.size.x / 2.f + CURSOR_DIST_FROM_TEXT, -size.y / 2.f));

	target_.draw(rect_);
	rect_.setOutlineThickness(RECT_OUTLINE_THICKNESS);
}
