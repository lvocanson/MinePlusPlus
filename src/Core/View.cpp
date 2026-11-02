#include "View.h"
#include <algorithm>
#include <cmath>
#include <numbers>

namespace
{

Vec2d rotated(Vec2d v, double cs, double sn)
{
	return {cs * v.x - sn * v.y, sn * v.x + cs * v.y};
}

} // namespace

Vec2d Transform::apply(Vec2d p) const
{
	return {a * p.x + b * p.y + c, d * p.x + e * p.y + f};
}

Transform View::toScreen() const
{
	double cs = std::cos(rotation) * zoom;
	double sn = std::sin(rotation) * zoom;
	Vec2d half = viewport / 2.0;
	return {
		cs, sn, half.x - (cs * center.x + sn * center.y),
		-sn, cs, half.y - (-sn * center.x + cs * center.y)
	};
}

Transform View::toWorld() const
{
	double cs = std::cos(rotation) / zoom;
	double sn = std::sin(rotation) / zoom;
	Vec2d half = viewport / 2.0;
	return {
		cs, -sn, center.x - (cs * half.x - sn * half.y),
		sn, cs, center.y - (sn * half.x + cs * half.y)
	};
}

void View::reset()
{
	center = viewport / 2.0;
	zoom = 1.0;
	rotation = 0.0;
}

bool View::fit(Rectd target)
{
	center = target.getCenter();
	if (viewport.x <= 0.0 || viewport.y <= 0.0 || target.size.x <= 0.0 || target.size.y <= 0.0)
		return true;
	double wanted = std::min(viewport.x / target.size.x, viewport.y / target.size.y);
	zoom = std::clamp(wanted, MIN_ZOOM, MAX_ZOOM);
	return zoom == wanted;
}

void View::pin(Vec2d world, Vec2d screen)
{
	center += world - toWorld().apply(screen);
}

void View::rotateAround(Vec2d pivot, double angle)
{
	center = pivot + rotated(center - pivot, std::cos(angle), std::sin(angle));
	rotation = std::remainder(rotation + angle, 2.0 * std::numbers::pi_v<double>);
}
