#pragma once
#include "Rect.h"
#include "Vec2.h"

struct Transform
{
	double a, b, c, d, e, f;

	Vec2d apply(Vec2d p) const;
};

struct View
{
	static constexpr double MIN_ZOOM = 1.0;
	static constexpr double MAX_ZOOM = 256.0;

	Vec2d center{};
	double zoom = 1.0;
	double rotation{};
	Vec2d viewport{};

	Transform toScreen() const;
	Transform toWorld() const;

	Vec2d getVisibleSize() const { return viewport / zoom; }

	void reset();
	bool fit(Rectd target);
	void pin(Vec2d world, Vec2d screen);
	void rotateAround(Vec2d pivot, double angle);
};
