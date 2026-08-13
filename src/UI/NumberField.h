#pragma once
#include <SFML/Graphics/Rect.hpp>
#include <limits>

struct NumberField
{
	sf::IntRect rect;
	static constexpr auto BUF_CAP = std::numeric_limits<std::size_t>::digits10 + 1;
	char32_t buffer[BUF_CAP];
	std::size_t bufferSz = 0;

	void add(char32_t c)
	{
		if (c == '\b' && bufferSz > 0)
		{
			--bufferSz;
			return;
		}

		if (c < U'0' || c > U'9' || bufferSz == BUF_CAP)
			return;

		buffer[bufferSz] = c;
		++bufferSz;

		constexpr std::size_t MAX = std::numeric_limits<std::size_t>::max();
		std::size_t value = 0;

		// Check if represented value is greater that max
		for (std::size_t i = 0; i < bufferSz; ++i)
		{
			std::size_t digit = std::size_t(buffer[i] - U'0');
			if (value > (MAX - digit) / 10)
			{
				// would overflow, change value to max
				setNumber(MAX);
				break;
			}
			value = value * 10 + digit;
		}
	}

	std::size_t toNumber() const
	{
		std::size_t value = 0;
		for (std::size_t i = 0; i < bufferSz; ++i)
			value = value * 10 + std::size_t(buffer[i] - U'0');
		return value;
	}

	void setNumber(std::size_t number)
	{
		char32_t tmp[BUF_CAP];
		std::size_t i = 0;

		do
		{
			tmp[i++] = char32_t(U'0' + (number % 10));
			number /= 10;
		} while (number != 0 && i < BUF_CAP);

		bufferSz = i;
		for (std::size_t j = 0; j < i; ++j)
			buffer[j] = tmp[i - j - 1];
	}

	std::u32string_view view() const
	{
		return {buffer, bufferSz};
	}
};
