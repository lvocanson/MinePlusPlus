#pragma once
#include "Vec2.h"
#include <cstdint>
#include <span>
#include <vector>

struct Image
{
	std::vector<std::uint32_t> pixels;
	Vec2i size;
};

Image loadPng(std::span<const unsigned char> bytes);
