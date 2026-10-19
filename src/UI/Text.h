#pragma once
#include "Core/Color.h"
#include "Core/Vec2.h"
#include <string_view>

struct Text
{
	enum Origin
	{
		TopLeft, Top, TopRight,
		Left, Middle, Right,
		BotLeft, Bot, BotRight,
	};

	Vec2i position;
	Origin origin = TopLeft;
	std::string_view string; // utf-8
	Color color = Colors::White;
};
