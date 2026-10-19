#pragma once
#include "Rect.h"
#include "Vec2.h"

struct Transform
{
	float a, b, c, d, e, f;

	Vec2f apply(Vec2f p) const;
};

struct View
{
	Vec2f center{};
	float zoom = 1.f;
	float rotation{};
	Vec2f viewport{};

	Transform toScreen() const;
	Transform toWorld() const;

	Vec2f getVisibleSize() const { return viewport / zoom; }

	void reset();
	void fit(Rectf target);
	void rotateAround(Vec2f pivot, float angle);
};
