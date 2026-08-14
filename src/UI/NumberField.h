#pragma once
#include <SFML/Graphics/Rect.hpp>
#include <limits>

struct NumberField
{
	sf::IntRect rect;
	std::size_t value;

	void add(char32_t c)
	{
		if (c == U'\b')
		{
			value /= 10;
			return;
		}

		if (c < U'0' || c > U'9')
			return;

		constexpr auto MAX = std::numeric_limits<std::size_t>::max();
		auto digit = std::size_t(c - U'0');
		value = value > (MAX - digit) / 10 ? MAX : value * 10 + digit;
	}
};
