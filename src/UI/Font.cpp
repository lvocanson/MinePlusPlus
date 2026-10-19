#include "Font.h"
#include <cstdlib>

Font::Font(std::span<const unsigned char> ttf, float pixelHeight)
	: info_{}
	, glyphs_{}
{
	if (!stbtt_InitFont(&info_, ttf.data(), stbtt_GetFontOffsetForIndex(ttf.data(), 0)))
		std::abort();

	scale_ = stbtt_ScaleForMappingEmToPixels(&info_, pixelHeight);

	int ascent{}, descent{}, lineGap{};
	stbtt_GetFontVMetrics(&info_, &ascent, &descent, &lineGap);
	lineSpacing_ = float(ascent - descent + lineGap) * scale_;

	for (char32_t codePoint = FIRST; codePoint <= LAST; ++codePoint)
	{
		Glyph& glyph = glyphs_[codePoint - FIRST];

		int advance{}, leftBearing{};
		stbtt_GetCodepointHMetrics(&info_, int(codePoint), &advance, &leftBearing);
		glyph.advance = float(advance) * scale_;

		int x0{}, y0{}, x1{}, y1{};
		stbtt_GetCodepointBitmapBox(&info_, int(codePoint), scale_, scale_, &x0, &y0, &x1, &y1);
		glyph.offset = {x0, y0};
		glyph.size = {x1 - x0, y1 - y0};
		glyph.bitmapOffset = std::uint32_t(bitmaps_.size());

		if (glyph.size.x <= 0 || glyph.size.y <= 0)
		{
			glyph.size = {};
			continue;
		}

		bitmaps_.resize(bitmaps_.size() + std::size_t(glyph.size.x) * std::size_t(glyph.size.y));
		stbtt_MakeCodepointBitmap(
			&info_,
			bitmaps_.data() + glyph.bitmapOffset,
			glyph.size.x,
			glyph.size.y,
			glyph.size.x,
			scale_,
			scale_,
			int(codePoint));
	}

	bitmaps_.shrink_to_fit();
}

const Glyph& Font::getGlyph(char32_t codePoint) const
{
	if (codePoint < FIRST || codePoint > LAST)
		codePoint = U'?';
	return glyphs_[codePoint - FIRST];
}

float Font::getKerning(char32_t first, char32_t second) const
{
	if (first == 0)
		return 0.f;
	return float(stbtt_GetCodepointKernAdvance(&info_, int(first), int(second))) * scale_;
}
