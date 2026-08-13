#include "App.h"
#include "Game/Resources.h"
#include "UI/UITarget.h"
#include "Utils/Overloaded.h"
#include <SFML/Graphics/RenderTexture.hpp>
#include <chrono>
#include <cstdlib>
#include <cmath>

namespace
{

sf::Vector2u getDefaultWindowSize()
{
	sf::Vector2u screenSize = sf::VideoMode::getDesktopMode().size;
	return sf::Vector2u(sf::Vector2f(screenSize) * 0.75f);
}

sf::ContextSettings getDefaultContextSettings()
{
	return sf::ContextSettings
	{
		.antiAliasingLevel = sf::RenderTexture::getMaximumAntiAliasingLevel()
	};
}

} // namespace

App::App()
	: window_(sf::VideoMode(getDefaultWindowSize()), "Mine++", sf::State::Windowed, getDefaultContextSettings())
	, clearColor_({0x31, 0x4D, 0x79, 0x00})
	, isMouseDraggingCamera_(false)
{
	window_.setVerticalSyncEnabled(true);
	std::visit([&](auto& ui) { ui(UIEvent::Resized{*this, sf::Vector2i(window_.getSize())}); }, ui_);
}

int App::run()
{
	auto lastFrame = std::chrono::steady_clock::now();
	while (window_.isOpen())
	{
		pollEvents();

		auto now = std::chrono::steady_clock::now();
		float dt = std::chrono::duration_cast<std::chrono::nanoseconds>(now - lastFrame).count() / 1e9f;
		lastFrame = now;
		game_.update(dt);

		window_.clear(clearColor_);
		{
			game_.render(window_);

			UITarget uiTarget(window_);
			Overloaded visitor
			{
				[](std::monostate&) {},
				[&](auto& ui) { ui.render(uiTarget); }
			};
			std::visit(visitor, ui_);
		}
		window_.display();

		processCommands();
	}

	return EXIT_SUCCESS;
}

void App::resetView()
{
	window_.setView(window_.getDefaultView());
}

void App::centerView(sf::FloatRect target)
{
	sf::Vector2f windowSize(window_.getSize());
	float windowAspect = windowSize.x / windowSize.y;
	float rectAspect = target.size.x / target.size.y;

	sf::Vector2f viewSize;
	if (rectAspect > windowAspect)
	{
		viewSize.x = target.size.x;
		viewSize.y = target.size.x / windowAspect;
	}
	else
	{
		viewSize.y = target.size.y;
		viewSize.x = target.size.y * windowAspect;
	}

	sf::View view = window_.getView();
	view.setCenter(target.getCenter());
	view.setSize(viewSize);
	window_.setView(view);
}

void App::pollEvents()
{
	Overloaded visitor
	{
		[&](const sf::Event::Closed& event)
		{
			window_.close();
		},

		[&](const sf::Event::Resized& event)
		{
			// Keep the whole previous view visible after resizing by zooming out if needed.
			const sf::View& view = window_.getView();
			sf::Vector2f center = view.getCenter();
			sf::Vector2f size = view.getSize();
			centerView({center - size / 2.f, size});
			std::visit([&](auto& ui) { ui(UIEvent::Resized{*this, sf::Vector2i(window_.getSize())}); }, ui_);
		},

		[&](const sf::Event::MouseButtonPressed& event)
		{
			if (event.button == sf::Mouse::Button::Middle)
			{
				isMouseDraggingCamera_ = true;
				return;
			}

			auto result = std::visit(
				[&](auto& ui) { return ui(UIEvent::Pressed{*this, event.button, event.position}); },
				ui_);
			if (result == UIEvent::Ignored)
			{
				sf::Vector2f position = window_.mapPixelToCoords(event.position);
				game_.dispatchWorldEvent(WorldEvent::Pressed{*this, event.button, position});
			}
		},

		[&](const sf::Event::MouseButtonReleased& event)
		{
			if (event.button == sf::Mouse::Button::Middle)
			{
				isMouseDraggingCamera_ = false;
				return;
			}

			auto result = std::visit(
				[&](auto& ui) { return ui(UIEvent::Released{*this, event.button, event.position}); },
				ui_);
			if (result == UIEvent::Ignored)
			{
				sf::Vector2f position = window_.mapPixelToCoords(event.position);
				game_.dispatchWorldEvent(WorldEvent::Released{*this, event.button, position});
			}
			else
			{
				// Any UI element that consumes a release acts as a click
				audio_.play(Resources::Sounds::click1);
			}
		},

		[&](const sf::Event::MouseWheelScrolled& event)
		{
			constexpr float sensitivity = -0.1f; // TODO: parameter + config file
			float factor = std::pow(2.f, event.delta * sensitivity);
			sf::View view = window_.getView();
			view.zoom(factor);
			sf::Vector2f position = window_.mapPixelToCoords(event.position);
			sf::Vector2f targetPos = window_.mapPixelToCoords(event.position, view);
			view.move(position - targetPos);
			window_.setView(view);
		},

		[&](const sf::Event::MouseMovedRaw& event)
		{
			if (isMouseDraggingCamera_)
			{
				constexpr float sensitivity = -1.f; // TODO: parameter + config file
				auto windowSize = sf::Vector2f(window_.getSize());
				// Assuming view and windowSize have the same aspect ratio
				float viewRatio = window_.getView().getSize().x / windowSize.x;
				auto delta = sensitivity * viewRatio * sf::Vector2f(event.delta);
				sf::View view = window_.getView();
				view.move(delta.rotatedBy(view.getRotation()));
				window_.setView(view);
			}
		},

		[&](const sf::Event::TextEntered& event)
		{
			std::visit([&](auto& ui) { ui(UIEvent::Typed{*this, event.unicode}); }, ui_);
		},

		[](const auto& ignored) {}
	};

	while (auto event = window_.pollEvent())
		event->visit(visitor);
}

void App::processCommands()
{
	Overloaded visitor
	{
		[](std::monostate&)
		{
			assert(false && "Never reached");
		},

		[&](RequestExit&)
		{
			window_.close();
		},

		[&](ChangeClearColor& cmd)
		{
			clearColor_ = cmd.color;
		},

		[&](SwapUI& cmd)
		{
			resetView();
			cmd.swapper(ui_);
			std::visit([&](auto& ui) { ui(UIEvent::Resized{*this, sf::Vector2i(window_.getSize())}); }, ui_);
		}
	};

	for (auto& command : commands_)
	{
		if (command.index() == 0)
			break;

		std::visit(visitor, command);
		command.emplace<0>();
	}
}
