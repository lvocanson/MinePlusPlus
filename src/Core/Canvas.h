#pragma once
#include "Color.h"
#include "Rect.h"
#include <algorithm>
#include <cstddef>
#include <cstdint>

struct Canvas
{
	std::uint32_t* pixels;
	Vec2i size;

	std::uint32_t* row(int y) const { return pixels + std::size_t(y) * std::size_t(size.x); }
	Recti bounds() const { return {{}, size}; }
};

Recti intersect(Recti lhs, Recti rhs);

std::uint32_t blend(std::uint32_t dst, Color src, unsigned coverage);

void fill(Canvas canvas, Color color);
void fillRect(Canvas canvas, Recti rect, Color color);
void strokeRect(Canvas canvas, Recti rect, int thickness, Color color);

template <class Coverage>
void blendCoverage(Canvas canvas, Recti area, Color color, Coverage&& coverage)
{
	Recti clipped = intersect(area, canvas.bounds());
	for (int y = clipped.position.y; y < clipped.position.y + clipped.size.y; ++y)
	{
		std::uint32_t* row = canvas.row(y);
		for (int x = clipped.position.x; x < clipped.position.x + clipped.size.x; ++x)
		{
			unsigned c = coverage(x - area.position.x, y - area.position.y);
			if (c != 0)
				row[x] = blend(row[x], color, c);
		}
	}
}
