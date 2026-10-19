#include "Canvas.h"

Recti intersect(Recti lhs, Recti rhs)
{
	int left = std::max(lhs.position.x, rhs.position.x);
	int top = std::max(lhs.position.y, rhs.position.y);
	int right = std::min(lhs.position.x + lhs.size.x, rhs.position.x + rhs.size.x);
	int bottom = std::min(lhs.position.y + lhs.size.y, rhs.position.y + rhs.size.y);
	return {{left, top}, {std::max(right - left, 0), std::max(bottom - top, 0)}};
}

std::uint32_t blend(std::uint32_t dst, Color src, unsigned coverage)
{
	unsigned alpha = coverage * src.a / 0xFF;
	if (alpha >= 0xFF)
		return src.toPixel();

	auto mix = [alpha](unsigned d, unsigned s) { return (s * alpha + d * (0xFF - alpha) + 0x7F) / 0xFF; };
	unsigned r = mix((dst >> 16) & 0xFF, src.r);
	unsigned g = mix((dst >> 8) & 0xFF, src.g);
	unsigned b = mix(dst & 0xFF, src.b);
	return (r << 16) | (g << 8) | b;
}

void fill(Canvas canvas, Color color)
{
	std::fill_n(canvas.pixels, std::size_t(canvas.size.x) * std::size_t(canvas.size.y), color.toPixel());
}

void fillRect(Canvas canvas, Recti rect, Color color)
{
	Recti clipped = intersect(rect, canvas.bounds());
	std::uint32_t pixel = color.toPixel();
	for (int y = clipped.position.y; y < clipped.position.y + clipped.size.y; ++y)
		std::fill_n(canvas.row(y) + clipped.position.x, clipped.size.x, pixel);
}

void strokeRect(Canvas canvas, Recti rect, int thickness, Color color)
{
	Vec2i outer = rect.size + Vec2i{thickness, thickness} * 2;
	Vec2i topLeft = rect.position - Vec2i{thickness, thickness};
	fillRect(canvas, {topLeft, {outer.x, thickness}}, color);
	fillRect(canvas, {{topLeft.x, rect.position.y + rect.size.y}, {outer.x, thickness}}, color);
	fillRect(canvas, {{topLeft.x, rect.position.y}, {thickness, rect.size.y}}, color);
	fillRect(canvas, {{rect.position.x + rect.size.x, rect.position.y}, {thickness, rect.size.y}}, color);
}
