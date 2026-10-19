#pragma once
#include "Core/Vec2.h"
#include <stb_truetype.h>
#include <array>
#include <cstdint>
#include <span>
#include <vector>

struct Glyph
{
	Vec2i offset;
	Vec2i size;
	float advance;
	std::uint32_t bitmapOffset;
};

// TODO: only printable ASCII is baked, anything else is drawn as '?'
class Font
{
public:

	Font(std::span<const unsigned char> ttf, float pixelHeight);

	const Glyph& getGlyph(char32_t codePoint) const;
	const std::uint8_t* getBitmap(const Glyph& glyph) const { return bitmaps_.data() + glyph.bitmapOffset; }
	float getKerning(char32_t first, char32_t second) const;
	float getLineSpacing() const { return lineSpacing_; }

private:

	static constexpr char32_t FIRST = U' ';
	static constexpr char32_t LAST = U'~';

	stbtt_fontinfo info_;
	float scale_;
	float lineSpacing_;
	std::array<Glyph, LAST - FIRST + 1> glyphs_;
	std::vector<std::uint8_t> bitmaps_;
};
