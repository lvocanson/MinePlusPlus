#include "ProfilerDraw.h"
#include "Game/Resources.h"
#include <cmath>
#include <cstdlib>

const Font& Profiling::Draw::font()
{
	return Resources::Fonts::roboto;
}

int Profiling::Draw::lineHeight()
{
	return int(std::ceil(font().getLineSpacing()));
}

float Profiling::Draw::textWidth(std::string_view text)
{
	float width = 0.f;
	char32_t previous = 0;
	for (char c : text)
	{
		width += font().getKerning(previous, char32_t(c)) + font().getGlyph(char32_t(c)).advance;
		previous = char32_t(c);
	}
	return width;
}

std::string_view Profiling::Draw::fitText(std::string_view text, float maxWidth)
{
	float width = 0.f;
	char32_t previous = 0;
	for (std::size_t i = 0; i < text.size(); ++i)
	{
		width += font().getKerning(previous, char32_t(text[i])) + font().getGlyph(char32_t(text[i])).advance;
		if (width > maxWidth)
			return text.substr(0, i);
		previous = char32_t(text[i]);
	}
	return text;
}

void Profiling::Draw::drawText(Canvas canvas, Vec2f baseline, std::string_view text, Color color)
{
	char32_t previous = 0;
	for (char c : text)
	{
		baseline.x += font().getKerning(previous, char32_t(c));
		previous = char32_t(c);

		Glyph glyph = font().getGlyph(char32_t(c));
		const std::uint8_t* bitmap = font().getBitmap(glyph);
		Vec2i topLeft = Vec2i{int(std::lround(baseline.x)), int(std::lround(baseline.y))} + glyph.offset;
		blendCoverage(
			canvas,
			{topLeft, glyph.size},
			color,
			[&](int x, int y) -> unsigned { return bitmap[y * glyph.size.x + x]; });

		baseline.x += glyph.advance;
	}
}

void Profiling::Draw::drawLine(Canvas canvas, Vec2i from, Vec2i to, Color color)
{
	std::uint32_t pixel = color.toPixel();
	auto plot = [&](int x, int y)
	{
		for (int py = y; py <= y + 1; ++py)
			if (x >= 0 && py >= 0 && x < canvas.size.x && py < canvas.size.y)
				canvas.row(py)[x] = pixel;
	};

	Vec2i delta{std::abs(to.x - from.x), -std::abs(to.y - from.y)};
	Vec2i step{from.x < to.x ? 1 : -1, from.y < to.y ? 1 : -1};
	int error = delta.x + delta.y;
	for (;;)
	{
		plot(from.x, from.y);
		if (from == to)
			break;
		int doubled = error * 2;
		if (doubled >= delta.y)
		{
			error += delta.y;
			from.x += step.x;
		}
		if (doubled <= delta.x)
		{
			error += delta.x;
			from.y += step.y;
		}
	}
}

Recti Profiling::Draw::drawButton(Canvas canvas, Vec2i position, std::string_view text, bool active)
{
	int height = lineHeight() + BUTTON_PADDING;
	Recti button{position, {BUTTON_PADDING * 2 + int(std::ceil(textWidth(text))), height}};
	fillRect(canvas, button, active ? ACTIVE_BUTTON_COLOR : BUTTON_COLOR);
	Vec2f baseline{float(position.x + BUTTON_PADDING), float(position.y + height / 2 + TEXT_BASELINE_OFFSET)};
	drawText(canvas, baseline, text, active ? TEXT_COLOR : MUTED_COLOR);
	return button;
}
