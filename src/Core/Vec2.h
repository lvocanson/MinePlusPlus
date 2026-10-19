#pragma once
#include <cstddef>

template <class T>
struct Vec2
{
	T x{}, y{};

	template <class U>
	constexpr explicit operator Vec2<U>() const { return {static_cast<U>(x), static_cast<U>(y)}; }

	constexpr bool operator==(const Vec2&) const = default;

	constexpr Vec2 operator-() const { return {-x, -y}; }

	constexpr Vec2 operator+(Vec2 rhs) const { return {x + rhs.x, y + rhs.y}; }
	constexpr Vec2 operator-(Vec2 rhs) const { return {x - rhs.x, y - rhs.y}; }
	constexpr Vec2 operator*(T rhs) const { return {x * rhs, y * rhs}; }
	constexpr Vec2 operator/(T rhs) const { return {x / rhs, y / rhs}; }

	constexpr Vec2& operator+=(Vec2 rhs) { x += rhs.x; y += rhs.y; return *this; }
	constexpr Vec2& operator-=(Vec2 rhs) { x -= rhs.x; y -= rhs.y; return *this; }
	constexpr Vec2& operator*=(T rhs) { x *= rhs; y *= rhs; return *this; }
	constexpr Vec2& operator/=(T rhs) { x /= rhs; y /= rhs; return *this; }

	friend constexpr Vec2 operator*(T lhs, Vec2 rhs) { return rhs * lhs; }
};

using Vec2i = Vec2<int>;
using Vec2f = Vec2<float>;
using Vec2u = Vec2<unsigned>;
using Vec2s = Vec2<std::size_t>;
using Vec2sDelta = Vec2<std::ptrdiff_t>;
