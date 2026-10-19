#include "Image.h"
#include <stb_image.h>
#include <cstdlib>

Image loadPng(std::span<const unsigned char> bytes)
{
	int width{}, height{}, channels{};
	stbi_uc* rgb = stbi_load_from_memory(bytes.data(), int(bytes.size()), &width, &height, &channels, 3);
	if (!rgb)
		std::abort();

	Image image{std::vector<std::uint32_t>(std::size_t(width) * std::size_t(height)), {width, height}};
	for (std::size_t i = 0; i < image.pixels.size(); ++i)
	{
		const stbi_uc* p = rgb + i * 3;
		image.pixels[i] = (std::uint32_t(p[0]) << 16) | (std::uint32_t(p[1]) << 8) | std::uint32_t(p[2]);
	}

	stbi_image_free(rgb);
	return image;
}
