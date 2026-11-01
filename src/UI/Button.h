#pragma once
#include "Core/Color.h"
#include "Core/Rect.h"
#include <string_view>

struct Button
{
	Recti rect;
	std::string_view text;
	Color textColor = Colors::White;
};
