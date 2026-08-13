#include "ClickTracker.h"
#include "Button.h"

bool ClickTracker::registerPress(sf::Vector2i position, std::initializer_list<std::reference_wrapper<Button>> buttons)
{
	for (Button& btn : buttons)
		if (btn.rect.contains(position))
		{
			pressed_ = &btn;
			return true;
		}
	return false;
}

bool ClickTracker::isClicked(Button& button, sf::Vector2i position)
{
	if (pressed_ == &button)
	{
		pressed_ = nullptr;
		return button.rect.contains(position);
	}
	return false;
}
