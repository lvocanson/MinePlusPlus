#pragma once
#include <cstdint>

std::uint32_t makeSystemSeed();

struct SeededRandom
{
	std::uint64_t state = 0;

	constexpr std::uint64_t next()
	{
		std::uint64_t x = state += 0x9E3779B97F4A7C15ull;
		x = (x ^ (x >> 30)) * 0xBF58476D1CE4E5B9ull;
		x = (x ^ (x >> 27)) * 0x94D049BB133111EBull;
		return x ^ (x >> 31);
	}

	constexpr std::uint32_t nextBelow(std::uint32_t range)
	{
		std::uint64_t product = (next() >> 32) * range;
		if (std::uint32_t(product) < range)
		{
			std::uint32_t threshold = (0u - range) % range;
			while (std::uint32_t(product) < threshold)
				product = (next() >> 32) * range;
		}
		return std::uint32_t(product >> 32);
	}
};
