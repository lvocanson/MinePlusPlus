#include <stb_image.h>
#include <stb_truetype.h>
#include <algorithm>
#include <charconv>
#include <cstdint>
#include <cstdio>
#include <format>
#include <fstream>
#include <iterator>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace
{

std::optional<std::vector<unsigned char>> readFile(const char* path)
{
	std::ifstream file(path, std::ios::binary);
	if (!file)
		return std::nullopt;
	return std::vector<unsigned char>(std::istreambuf_iterator<char>(file), {});
}

bool writeIfChanged(const char* path, const std::string& content)
{
	if (auto existing = readFile(path); existing && std::string_view(reinterpret_cast<const char*>(existing->data()), existing->size()) == content)
		return true;

	std::ofstream file(path, std::ios::binary | std::ios::trunc);
	file << content;
	return bool(file);
}

template <class T>
void appendArray(std::string& out, std::string_view type, std::string_view name, const std::vector<T>& values, auto&& format)
{
	std::format_to(std::back_inserter(out), "constexpr {} {}[] =\n{{", type, name);
	for (std::size_t i = 0; i < values.size(); ++i)
	{
		out += i % 16 == 0 ? "\n\t" : " ";
		format(out, values[i]);
		out += ',';
	}
	out += "\n};\n\n";
}

std::string floatLiteral(float value)
{
	return value == 0.f ? "0.f" : std::format("{:a}f", value).insert(value < 0.f ? 1 : 0, "0x");
}

std::string headerFor(std::string_view name, std::string_view type)
{
	return std::format(
		"#pragma once\n#include <BakedTypes.h>\nnamespace assets {{ const {}& {}() noexcept; }}\n",
		type,
		name);
}

int bakeAtlas(const char* input, const char* header, const char* source, std::string_view name)
{
	auto png = readFile(input);
	if (!png)
		return std::fprintf(stderr, "asset_baker: cannot read %s\n", input), 1;

	int width{}, height{}, channels{};
	stbi_uc* rgb = stbi_load_from_memory(png->data(), int(png->size()), &width, &height, &channels, 3);
	if (!rgb)
		return std::fprintf(stderr, "asset_baker: cannot decode %s\n", input), 1;

	std::vector<std::uint32_t> palette;
	std::vector<std::uint8_t> indices(std::size_t(width) * std::size_t(height));
	for (std::size_t i = 0; i < indices.size(); ++i)
	{
		const stbi_uc* p = rgb + i * 3;
		std::uint32_t color = (std::uint32_t(p[0]) << 16) | (std::uint32_t(p[1]) << 8) | std::uint32_t(p[2]);
		auto it = std::ranges::find(palette, color);
		if (it == palette.end())
		{
			if (palette.size() == 256)
				return std::fprintf(stderr, "asset_baker: %s has more than 256 colors\n", input), 1;
			it = palette.insert(palette.end(), color);
		}
		indices[i] = std::uint8_t(it - palette.begin());
	}
	stbi_image_free(rgb);

	std::string out = std::format("#include \"{}.h\"\n\nnamespace\n{{\n\n", name);
	appendArray(out, "std::uint32_t", "palette", palette, [](std::string& s, std::uint32_t v) { std::format_to(std::back_inserter(s), "0x{:06X}", v); });
	appendArray(out, "std::uint8_t", "indices", indices, [](std::string& s, std::uint8_t v) { std::format_to(std::back_inserter(s), "{}", v); });
	std::format_to(
		std::back_inserter(out),
		"}} // namespace\n\nconst assets::IndexedImage& assets::{}() noexcept\n{{\n"
		"\tstatic constexpr IndexedImage image{{{}, {}, palette, indices}};\n\treturn image;\n}}\n",
		name,
		width,
		height);

	return writeIfChanged(header, headerFor(name, "IndexedImage")) && writeIfChanged(source, out) ? 0 : 1;
}

struct Glyph
{
	int offsetX, offsetY, width, height;
	float advance;
	std::size_t bitmapOffset;
};

struct Kerning
{
	char32_t first, second;
	float advance;
};

int bakeFont(const char* input, const char* header, const char* source, std::string_view name, float pixelHeight)
{
	constexpr char32_t FIRST = U' ';
	constexpr char32_t LAST = U'~';

	auto ttf = readFile(input);
	stbtt_fontinfo info{};
	if (!ttf || !stbtt_InitFont(&info, ttf->data(), stbtt_GetFontOffsetForIndex(ttf->data(), 0)))
		return std::fprintf(stderr, "asset_baker: cannot load font %s\n", input), 1;

	float scale = stbtt_ScaleForMappingEmToPixels(&info, pixelHeight);
	int ascent{}, descent{}, lineGap{};
	stbtt_GetFontVMetrics(&info, &ascent, &descent, &lineGap);

	std::vector<Glyph> glyphs;
	std::vector<std::uint8_t> bitmaps;
	for (char32_t codePoint = FIRST; codePoint <= LAST; ++codePoint)
	{
		int advance{}, leftBearing{};
		stbtt_GetCodepointHMetrics(&info, int(codePoint), &advance, &leftBearing);
		int x0{}, y0{}, x1{}, y1{};
		stbtt_GetCodepointBitmapBox(&info, int(codePoint), scale, scale, &x0, &y0, &x1, &y1);

		Glyph glyph{x0, y0, x1 - x0, y1 - y0, float(advance) * scale, bitmaps.size()};
		if (glyph.width <= 0 || glyph.height <= 0)
		{
			glyph.width = glyph.height = 0;
		}
		else
		{
			bitmaps.resize(bitmaps.size() + std::size_t(glyph.width) * std::size_t(glyph.height));
			stbtt_MakeCodepointBitmap(&info, bitmaps.data() + glyph.bitmapOffset, glyph.width, glyph.height, glyph.width, scale, scale, int(codePoint));
		}
		glyphs.push_back(glyph);
	}

	std::vector<Kerning> kerning;
	for (char32_t first = FIRST; first <= LAST; ++first)
		for (char32_t second = FIRST; second <= LAST; ++second)
			if (int value = stbtt_GetCodepointKernAdvance(&info, int(first), int(second)); value != 0)
				kerning.push_back({first, second, float(value) * scale});

	std::string out = std::format("#include \"{}.h\"\n\nnamespace\n{{\n\n", name);
	appendArray(out, "assets::BakedGlyph", "glyphs", glyphs, [](std::string& s, const Glyph& g)
	{
		std::format_to(std::back_inserter(s), "{{{}, {}, {}, {}, {}, {}}}", g.offsetX, g.offsetY, g.width, g.height, floatLiteral(g.advance), g.bitmapOffset);
	});
	appendArray(out, "std::uint8_t", "bitmaps", bitmaps, [](std::string& s, std::uint8_t v) { std::format_to(std::back_inserter(s), "{}", v); });
	appendArray(out, "assets::KerningPair", "kerning", kerning, [](std::string& s, const Kerning& k)
	{
		std::format_to(std::back_inserter(s), "{{{}, {}, {}}}", std::uint32_t(k.first), std::uint32_t(k.second), floatLiteral(k.advance));
	});
	std::format_to(
		std::back_inserter(out),
		"}} // namespace\n\nconst assets::BakedFont& assets::{}() noexcept\n{{\n"
		"\tstatic constexpr BakedFont font{{{}, {}, glyphs, bitmaps, kerning}};\n\treturn font;\n}}\n",
		name,
		floatLiteral(float(ascent - descent + lineGap) * scale),
		std::uint32_t(FIRST));

	return writeIfChanged(header, headerFor(name, "BakedFont")) && writeIfChanged(source, out) ? 0 : 1;
}

} // namespace

int main(int argc, char** argv)
{
	std::string_view command = argc > 1 ? argv[1] : "";
	if (command == "atlas" && argc == 6)
		return bakeAtlas(argv[2], argv[3], argv[4], argv[5]);

	if (command == "font" && argc == 7)
	{
		float pixelHeight{};
		std::string_view height = argv[6];
		if (std::from_chars(height.data(), height.data() + height.size(), pixelHeight).ec == std::errc{})
			return bakeFont(argv[2], argv[3], argv[4], argv[5], pixelHeight);
	}

	std::fprintf(stderr, "usage: asset_baker atlas <png> <header> <source> <name>\n"
	                     "       asset_baker font <ttf> <header> <source> <name> <pixelHeight>\n");
	return 2;
}
