#pragma once
#include <SFML/System/Vector2.hpp>
#include <string>

struct Text
{
	enum Origin
	{
		TopLeft, Top, TopRight,
		Left, Middle, Right,
		BotLeft, Bot, BotRight,
	};

	sf::Vector2i position;
	Origin origin = TopLeft;
	std::string string;
};
