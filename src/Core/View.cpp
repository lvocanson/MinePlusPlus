#include "View.h"
#include <algorithm>
#include <cmath>
#include <numbers>

namespace
{

Vec2f rotated(Vec2f v, float cs, float sn)
{
	return {cs * v.x - sn * v.y, sn * v.x + cs * v.y};
}

} // namespace

Vec2f Transform::apply(Vec2f p) const
{
	return {a * p.x + b * p.y + c, d * p.x + e * p.y + f};
}

Transform View::toScreen() const
{
	float cs = std::cos(rotation) * zoom;
	float sn = std::sin(rotation) * zoom;
	Vec2f half = viewport / 2.f;
	return {
		cs, sn, half.x - (cs * center.x + sn * center.y),
		-sn, cs, half.y - (-sn * center.x + cs * center.y)
	};
}

Transform View::toWorld() const
{
	float cs = std::cos(rotation) / zoom;
	float sn = std::sin(rotation) / zoom;
	Vec2f half = viewport / 2.f;
	return {
		cs, -sn, center.x - (cs * half.x - sn * half.y),
		sn, cs, center.y - (sn * half.x + cs * half.y)
	};
}

void View::reset()
{
	center = viewport / 2.f;
	zoom = 1.f;
	rotation = 0.f;
}

void View::fit(Rectf target)
{
	center = target.getCenter();
	if (viewport.x <= 0.f || viewport.y <= 0.f || target.size.x <= 0.f || target.size.y <= 0.f)
		return;
	zoom = std::min(viewport.x / target.size.x, viewport.y / target.size.y);
}

void View::rotateAround(Vec2f pivot, float angle)
{
	center = pivot + rotated(center - pivot, std::cos(angle), std::sin(angle));
	rotation = std::remainder(rotation + angle, 2.f * std::numbers::pi_v<float>);
}
