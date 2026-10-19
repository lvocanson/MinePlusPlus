#pragma once
#include "Vec2.h"

template <class T>
struct Rect
{
	Vec2<T> position, size;

	template <class U>
	constexpr explicit operator Rect<U>() const { return {Vec2<U>(position), Vec2<U>(size)}; }

	constexpr bool operator==(const Rect&) const = default;

	constexpr bool contains(Vec2<T> point) const
	{
		return point.x >= position.x && point.x < position.x + size.x
		    && point.y >= position.y && point.y < position.y + size.y;
	}

	constexpr Vec2<T> getCenter() const { return position + size / T(2); }
};

using Recti = Rect<int>;
using Rectf = Rect<float>;
