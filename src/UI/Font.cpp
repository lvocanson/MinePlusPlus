#include "Font.h"
#include <algorithm>
#include <utility>

Glyph Font::getGlyph(char32_t codePoint) const
{
	std::size_t index = codePoint - baked_.firstCodePoint;
	if (codePoint < baked_.firstCodePoint || index >= baked_.glyphs.size())
		index = U'?' - baked_.firstCodePoint;

	const assets::BakedGlyph& glyph = baked_.glyphs[index];
	return {{glyph.offsetX, glyph.offsetY}, {glyph.width, glyph.height}, glyph.advance, glyph.bitmapOffset};
}

float Font::getKerning(char32_t first, char32_t second) const
{
	auto key = [](const assets::KerningPair& pair) { return std::pair{pair.first, pair.second}; };
	auto it = std::ranges::lower_bound(baked_.kerning, std::pair{first, second}, {}, key);
	return it != baked_.kerning.end() && it->first == first && it->second == second ? it->advance : 0.f;
}
