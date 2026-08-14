#pragma once
#include <SFML/Graphics/Rect.hpp>
#include <string_view>

struct Button
{
	sf::IntRect rect;
	std::string_view text;
};
