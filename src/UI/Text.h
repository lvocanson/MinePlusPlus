#pragma once
#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Vector2.hpp>
#include <string_view>

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
	std::string_view string; // utf-8
	sf::Color color = sf::Color::White;
};
