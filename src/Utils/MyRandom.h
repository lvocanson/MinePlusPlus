#pragma once
#include <chrono>
#include <cstdint>
#include <random>

namespace MyRandom
{

constexpr std::uint64_t mix(std::uint64_t x)
{
	x += 0x9E3779B97F4A7C15ull;
	x = (x ^ (x >> 30)) * 0xBF58476D1CE4E5B9ull;
	x = (x ^ (x >> 27)) * 0x94D049BB133111EBull;
	return x ^ (x >> 31);
}

inline std::uint64_t makeSeed()
{
	static const char addressProbe{};
	auto ticks = std::uint64_t(std::chrono::steady_clock::now().time_since_epoch().count());
	auto wall = std::uint64_t(std::chrono::system_clock::now().time_since_epoch().count());
	return mix(ticks ^ mix(wall ^ mix(reinterpret_cast<std::uintptr_t>(&addressProbe))));
}

} // namespace MyRandom

// 64 bits number generator
inline std::mt19937_64 gen(MyRandom::makeSeed());
