#pragma once
#include <cstdint>
#include <span>

namespace assets
{

struct IndexedImage
{
	int width;
	int height;
	std::span<const std::uint32_t> palette;
	std::span<const std::uint8_t> indices;
};

struct BakedGlyph
{
	std::int16_t offsetX;
	std::int16_t offsetY;
	std::uint16_t width;
	std::uint16_t height;
	float advance;
	std::uint32_t bitmapOffset;
};

struct KerningPair
{
	char32_t first;
	char32_t second;
	float advance;
};

struct BakedFont
{
	float lineSpacing;
	char32_t firstCodePoint;
	std::span<const BakedGlyph> glyphs;
	std::span<const std::uint8_t> bitmaps;
	std::span<const KerningPair> kerning;
};

} // namespace assets
