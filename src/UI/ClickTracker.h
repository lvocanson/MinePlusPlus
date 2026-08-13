#pragma once
#include <SFML/System/Vector2.hpp>
#include <functional>
#include <initializer_list>

struct Button;

class ClickTracker
{
	Button* pressed_{};

public:

	bool registerPress(sf::Vector2i position, std::initializer_list<std::reference_wrapper<Button>> buttons);
	bool isClicked(Button& button, sf::Vector2i position);
};
