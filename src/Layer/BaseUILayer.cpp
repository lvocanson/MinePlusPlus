#include "BaseUILayer.h"
#include "App.h"

struct BaseUILayer::EventDispatcher
{
	EventConsumed operator()(const sf::Event::Closed& event)
	{
		App::instance().exit();
		return EventConsumed::Yes;
	}

	EventConsumed operator()(const sf::Event::Resized& event)
	{
		// Change the view aspect ratio to match the window
		auto size = sf::Vector2f(event.size);
		sf::View view = App::instance().getView();
		sf::Vector2f viewSize = view.getSize();
		float ratio = (viewSize.x * size.y) / (size.x * viewSize.y);
		viewSize.y *= ratio;
		view.setSize(viewSize);
		App::instance().setView(view);
		return EventConsumed::Yes;
	}

	EventConsumed operator()(const sf::Event::MouseMovedRaw& event)
	{
		if (self.isMouseDraggingCamera_)
		{
			constexpr float sensitivity = 1.f;
			sf::View view = App::instance().getView();
			auto windowSize = sf::Vector2f(App::instance().getWindowSize());
			// Assuming view and windowSize have the same aspect ratio
			float viewRatio = view.getSize().x / windowSize.x;
			auto offset = -(sensitivity * viewRatio) * sf::Vector2f(event.delta);
			view.move(offset);
			App::instance().setView(view);
			return EventConsumed::Yes;
		}
		return EventConsumed::No;
	}

	EventConsumed operator()(const sf::Event::MouseButtonPressed& event)
	{
		sf::Vector2f position = App::instance().screenToWorld(event.position);
		return self.onPress(event.button, position);
	}

	EventConsumed operator()(const sf::Event::MouseButtonReleased& event)
	{
		sf::Vector2f position = App::instance().screenToWorld(event.position);
		return self.onRelease(event.button, position);
	}

	EventConsumed operator()(const sf::Event::MouseWheelScrolled& event)
	{
		if (self.isMouseWheelControllingZoom_)
		{
			constexpr float sensitivity = -0.1f;
			float factor = std::pow(2.f, event.delta * sensitivity);
			sf::View view = App::instance().getView();
			view.setSize(view.getSize() * factor);
			App::instance().setView(view);
			return EventConsumed::Yes;
		}
		return EventConsumed::No;
	}

	EventConsumed operator()(const auto& ignored)
	{
		return EventConsumed::No;
	}

	BaseUILayer& self;
};

EventConsumed BaseUILayer::listenEvent(const sf::Event& event)
{
	return event.visit(EventDispatcher{*this});
}

