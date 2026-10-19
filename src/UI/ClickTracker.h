#pragma once
#include "Core/Vec2.h"
#include <functional>
#include <initializer_list>

struct Button;

class ClickTracker
{
	Button* pressed_{};

public:

	bool registerPress(Vec2i position, std::initializer_list<std::reference_wrapper<Button>> buttons);
	bool isClicked(Button& button, Vec2i position);
};
