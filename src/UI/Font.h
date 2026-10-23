#pragma once
#include "Core/Vec2.h"
#include <BakedTypes.h>
#include <cstdint>

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

	explicit Font(const assets::BakedFont& baked) : baked_(baked) {}

	Glyph getGlyph(char32_t codePoint) const;
	const std::uint8_t* getBitmap(const Glyph& glyph) const { return baked_.bitmaps.data() + glyph.bitmapOffset; }
	float getKerning(char32_t first, char32_t second) const;
	float getLineSpacing() const { return baked_.lineSpacing; }

private:

	const assets::BakedFont& baked_;
};
