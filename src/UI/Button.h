#pragma once
#include "Core/Rect.h"
#include <string_view>

struct Button
{
	Recti rect;
	std::string_view text;
};
