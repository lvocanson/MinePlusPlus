#include "WindowInput.h"
#include "App.h"
#include "Utils/Overloaded.h"

EventConsumed WindowInput::handleEvent(const sf::Event& event)
{
	return event.visit(Overloaded
		{
			[](const sf::Event::Closed & event)
			{
				App::instance().exit();
				return EventConsumed::Yes;
			},

			[](const sf::Event::Resized & event)
			{
				// Change the view aspect ratio to match the window
				auto size = sf::Vector2f(event.size);
				sf::View view = App::instance().window.getView();
				sf::Vector2f viewSize = view.getSize();
				float ratio = (viewSize.x * size.y) / (size.x * viewSize.y);
				viewSize.y *= ratio;
				view.setSize(viewSize);
				App::instance().window.setView(view);
				return EventConsumed::Yes;
			},

			[](const auto&)
			{
				return EventConsumed::No;
			}
		});
}

