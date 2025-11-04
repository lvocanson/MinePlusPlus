#include "MainMenu.h"
#include "Core/App.h"

MainMenu::MainMenu()
{
	App::instance().clearColor = {0x31, 0x4D, 0x79, 0x00};
}

EventConsumed MainMenu::handleEvent(const sf::Event& event)
{
	return EventConsumed();
}

void MainMenu::render(sf::RenderTarget& target) const
{
}
