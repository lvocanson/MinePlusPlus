#include "UITarget.h"
#include "Game/Resources.h"
#include "Button.h"
#include "NumberField.h"
#include "Cursor.h"
#include <algorithm>
#include <cmath>

namespace
{

constexpr auto& FONT_REF = Resources::Fonts::roboto;
constexpr int OUTLINE_THICKNESS = 1;
constexpr int BOLD_WEIGHT = 1;
constexpr Color FILL_COLOR = Colors::White;
constexpr Color OUTLINE_COLOR = Colors::Black;
constexpr int CURSOR_WIDTH = 2;
constexpr int CURSOR_DIST_FROM_TEXT = 2;
constexpr int RECT_OUTLINE_THICKNESS = 2;
constexpr Color RECT_OUTLINE_COLOR = Colors::White;
constexpr Color BUTTON_COLOR = {0x79, 0xB6, 0x1E};

struct GlyphLayout
{
	int bold{};
	Vec2f pen{};
	char32_t previous{};
	Vec2f inkMin{}, inkMax{};
	bool hasInk{};

	// Calls sink(glyph, pen) for each code point that has a bitmap.
	template <class Sink>
	void put(char32_t codePoint, Sink&& sink)
	{
		if (codePoint == U'\r')
			return;

		pen.x += FONT_REF.getKerning(previous, codePoint);
		previous = codePoint;

		switch (codePoint)
		{
		case U'\n': pen = {0.f, pen.y + FONT_REF.getLineSpacing()};
			return;
		case U'\t': pen.x += FONT_REF.getGlyph(U' ').advance * 4.f;
			return;
		default: break;
		}

		const Glyph& glyph = FONT_REF.getGlyph(codePoint);

		// Whitespace has an advance but nothing to rasterize
		if (glyph.size.x != 0 && glyph.size.y != 0)
		{
			Vec2f min = pen + Vec2f(glyph.offset);
			Vec2f max = min + Vec2f(glyph.size) + Vec2f{float(bold), 0.f};
			inkMin = hasInk ? Vec2f{std::min(inkMin.x, min.x), std::min(inkMin.y, min.y)} : min;
			inkMax = hasInk ? Vec2f{std::max(inkMax.x, max.x), std::max(inkMax.y, max.y)} : max;
			hasInk = true;
			sink(glyph, pen);
		}

		pen.x += glyph.advance + float(bold);
	}

	Rectf bounds() const
	{
		if (!hasInk)
			return {};

		Vec2f outline{float(OUTLINE_THICKNESS), float(OUTLINE_THICKNESS)};
		return {inkMin - outline, (inkMax - inkMin) + outline * 2.f};
	}
};

Vec2f anchorOf(const Rectf& bounds, Text::Origin origin)
{
	switch (origin)
	{
	case Text::TopLeft: return bounds.position;
	case Text::Top: return {bounds.position.x + bounds.size.x / 2.f, bounds.position.y};
	case Text::TopRight: return bounds.position + Vec2f{bounds.size.x, 0.f};
	case Text::Left: return {bounds.position.x, bounds.position.y + bounds.size.y / 2.f};
	case Text::Middle: return bounds.getCenter();
	case Text::Right: return {bounds.position.x + bounds.size.x, bounds.position.y + bounds.size.y / 2.f};
	case Text::BotLeft: return bounds.position + Vec2f{0.f, bounds.size.y};
	case Text::Bot: return {bounds.position.x + bounds.size.x / 2.f, bounds.position.y + bounds.size.y};
	case Text::BotRight: return bounds.position + bounds.size;
	default: return {};
	}
}

char32_t decodeUtf8(std::string_view::iterator& it, std::string_view::iterator end)
{
	constexpr char32_t REPLACEMENT = 0xFFFD;

	auto lead = static_cast<unsigned char>(*it++);
	int continuations = lead < 0x80 ? 0
	                    : (lead >> 5) == 0x06 ? 1
	                    : (lead >> 4) == 0x0E ? 2
	                    : (lead >> 3) == 0x1E ? 3
	                    : -1;
	if (continuations < 0)
		return REPLACEMENT;

	char32_t codePoint = continuations == 0 ? lead : lead & (0x3F >> continuations);
	for (int i = 0; i < continuations; ++i, ++it)
	{
		if (it == end || (static_cast<unsigned char>(*it) & 0xC0) != 0x80)
			return REPLACEMENT;
		codePoint = (codePoint << 6) | (static_cast<unsigned char>(*it) & 0x3F);
	}
	return codePoint;
}

// Decodes UTF-8 on the fly, without any intermediate buffer.
auto codePointsOf(std::string_view utf8)
{
	return [utf8](auto&& put)
	{
		for (auto it = utf8.begin(); it != utf8.end();)
			put(decodeUtf8(it, utf8.end()));
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
Rectf textSize(Sequence&& sequence, int bold = 0)
{
	GlyphLayout layout{.bold = bold};
	sequence(
		[&](char32_t codePoint)
		{
			layout.put(codePoint, [](const Glyph&, Vec2f) {});
		});
	return layout.bounds();
}

Vec2i glyphTopLeft(const Glyph& glyph, Vec2f pen)
{
	return Vec2i{int(std::lround(pen.x)), int(std::lround(pen.y))} + glyph.offset;
}

// TODO: square dilation, a round stroke would need a distance field
void drawGlyph(Canvas canvas, const Glyph& glyph, Vec2i topLeft, Vec2i before, Vec2i after, Color color)
{
	const std::uint8_t* bitmap = FONT_REF.getBitmap(glyph);
	Vec2i grow = before + after;

	blendCoverage(
		canvas,
		{topLeft - before, glyph.size + grow},
		color,
		[&](int x, int y) -> unsigned
		{
			int left = std::max(x - grow.x, 0), right = std::min(x, glyph.size.x - 1);
			int top = std::max(y - grow.y, 0), bottom = std::min(y, glyph.size.y - 1);

			unsigned coverage = 0;
			for (int sy = top; sy <= bottom; ++sy)
				for (int sx = left; sx <= right; ++sx)
					coverage = std::max<unsigned>(coverage, bitmap[sy * glyph.size.x + sx]);
			return coverage;
		});
}

} // namespace

template <class Sequence>
void UITarget::pushTextAt(Sequence&& sequence, Vec2f offset, Color color, bool bold)
{
	int weight = bold ? BOLD_WEIGHT : 0;
	Vec2i outlineBefore{OUTLINE_THICKNESS, OUTLINE_THICKNESS};
	Vec2i outlineAfter = outlineBefore + Vec2i{weight, 0};

	GlyphLayout outlineLayout{.bold = weight};
	sequence(
		[&](char32_t codePoint)
		{
			outlineLayout.put(
				codePoint,
				[&](const Glyph& glyph, Vec2f pen)
				{
					drawGlyph(canvas_, glyph, glyphTopLeft(glyph, pen + offset), outlineBefore, outlineAfter, OUTLINE_COLOR);
				});
		});

	GlyphLayout fillLayout{.bold = weight};
	sequence(
		[&](char32_t codePoint)
		{
			fillLayout.put(
				codePoint,
				[&](const Glyph& glyph, Vec2f pen)
				{
					drawGlyph(canvas_, glyph, glyphTopLeft(glyph, pen + offset), {}, {weight, 0}, color);
				});
		});
}

template <class Sequence>
void UITarget::pushText(Sequence&& sequence, Vec2f position, Text::Origin origin, Color color, bool bold)
{
	Rectf bounds = textSize(sequence, bold ? BOLD_WEIGHT : 0);
	pushTextAt(sequence, position - anchorOf(bounds, origin), color, bold);
}

void UITarget::draw(const Button& button)
{
	fillRect(canvas_, button.rect, BUTTON_COLOR);
	strokeRect(canvas_, button.rect, RECT_OUTLINE_THICKNESS, RECT_OUTLINE_COLOR);

	pushText(codePointsOf(button.text), Rectf(button.rect).getCenter(), Text::Middle, button.textColor);
}

void UITarget::draw(const Text& text)
{
	if (!text.centerLines)
	{
		pushText(codePointsOf(text.string), Vec2f(text.position), text.origin, text.color, text.bold);
		return;
	}

	int weight = text.bold ? BOLD_WEIGHT : 0;
	Rectf block = textSize(codePointsOf(text.string), weight);
	Vec2f offset = Vec2f(text.position) - anchorOf(block, text.origin);
	float centerX = offset.x + block.getCenter().x;

	std::string_view rest = text.string;
	for (float lineY = offset.y;; lineY += FONT_REF.getLineSpacing())
	{
		std::size_t end = rest.find('\n');
		std::string_view line = rest.substr(0, end);
		Rectf bounds = textSize(codePointsOf(line), weight);
		pushTextAt(codePointsOf(line), {centerX - bounds.getCenter().x, lineY}, text.color, text.bold);
		if (end == std::string_view::npos)
			break;
		rest.remove_prefix(end + 1);
	}
}

void UITarget::draw(const NumberField& field)
{
	fillRect(canvas_, field.rect, Colors::White);
	strokeRect(canvas_, field.rect, RECT_OUTLINE_THICKNESS, RECT_OUTLINE_COLOR);

	pushText(codePointsOf(field.value), Rectf(field.rect).getCenter(), Text::Middle, FILL_COLOR);
}

void UITarget::draw(const Cursor& cursor)
{
	if (!cursor.field)
		return;

	Rectf fieldRect(cursor.field->rect);
	Rectf bounds = textSize(codePointsOf(cursor.field->value));

	Vec2f size{float(CURSOR_WIDTH), FONT_REF.getLineSpacing() * 0.9f};
	Vec2f position = fieldRect.getCenter() + Vec2f{bounds.size.x / 2.f + float(CURSOR_DIST_FROM_TEXT), -size.y / 2.f};
	fillRect(canvas_, Recti{Vec2i(position), Vec2i(size)}, Colors::Black);
}
