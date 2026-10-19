#pragma once
#include <cstdint>

struct Color
{
	std::uint8_t r{}, g{}, b{}, a = 0xFF;

	constexpr bool operator==(const Color&) const = default;

	constexpr std::uint32_t toPixel() const
	{
		return (std::uint32_t(r) << 16) | (std::uint32_t(g) << 8) | std::uint32_t(b);
	}
};

namespace Colors
{

inline constexpr Color White{0xFF, 0xFF, 0xFF};
inline constexpr Color Black{0x00, 0x00, 0x00};

} // namespace Colors
