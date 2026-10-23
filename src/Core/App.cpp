#include "App.h"
#include "Game/Resources.h"
#include "UI/UITarget.h"
#include "Utils/Overloaded.h"
#include <chrono>
#include <cstdlib>
#include <cmath>

#ifdef MPP_PROFILING
#include "Profiling/Profiler.h"
#endif

App::App()
	: clearColor_{0x31, 0x4D, 0x79}
	, isMouseDraggingCamera_(false)
	, mousePos_{}
{
	window_.open(false);
	if (window_.isOpen())
		setupEvents();

	view_.viewport = Vec2f(window_.getSize());
	view_.reset();
	std::visit([&](auto& ui) { ui(UIEvent::Resized{*this, window_.getSize()}); }, ui_);
}

int App::run()
{
	auto lastFrame = std::chrono::steady_clock::now();
	while (window_.isOpen())
	{
		auto now = std::chrono::steady_clock::now();
		float dt = std::chrono::duration_cast<std::chrono::nanoseconds>(now - lastFrame).count() / 1e9f;
		lastFrame = now;
		game_.update(view_, dt);

		Canvas canvas = window_.beginFrame(clearColor_);
		{
			game_.render(canvas, view_);

			UITarget uiTarget(canvas);
			auto visitor = [&](auto& ui) { ui.render(uiTarget); };
			std::visit(visitor, ui_);
		}
		window_.display();

#ifdef MPP_PROFILING
		Profiling::update(window_.getHandle(), dt);
#endif

		processCommands();
	}

#ifdef MPP_PROFILING
	Profiling::shutdown();
#endif

	audio_.waitUntilIdle(Audio::Track::Effect, std::chrono::seconds(2));
	return EXIT_SUCCESS;
}

void App::resetView()
{
	view_.reset();
}

void App::setViewTarget(Rectf target)
{
	view_.fit(target);
}

void App::setupEvents()
{
	auto hwnd = window_.getHandle();
	mfb_set_resize_callback(
		[this](mfb_window*, int width, int height)
		{
			// Keep the whole previous view visible after resizing by zooming out if needed.
			Vec2f visible = view_.getVisibleSize();
			view_.viewport = Vec2f(Vec2i{width, height});
			view_.fit({view_.center - visible / 2.f, visible});
			std::visit([&](auto& ui) { ui(UIEvent::Resized{*this, Vec2i{width, height}}); }, ui_);
		},
		hwnd);

	mfb_set_mouse_button_callback(
		[this](mfb_window*, mfb_mouse_button btn, mfb_key_mod, bool pressed)
		{
			if (btn == MFB_MOUSE_MIDDLE)
			{
				isMouseDraggingCamera_ = pressed;
				return;
			}

			UIEvent event = pressed
			                ? UIEvent{UIEvent::Pressed{*this, btn, mousePos_}}
			                : UIEvent{UIEvent::Released{*this, btn, mousePos_}};
			auto result = std::visit([&](auto& ui) { return event.visit(ui); }, ui_);
			if (result == UIEvent::Ignored)
			{
				Vec2f position = view_.toWorld().apply(Vec2f(mousePos_));
				game_.dispatchWorldEvent(
					pressed
					? WorldEvent{WorldEvent::Pressed{*this, btn, position}}
					: WorldEvent{WorldEvent::Released{*this, btn, position}});
			}
			else if (!pressed)
			{
				// Any UI element that consumes a release acts as a click
				audio_.play(Resources::Sounds::click1);
			}
		},
		hwnd);

	mfb_set_mouse_scroll_callback(
		[this](mfb_window*, mfb_key_mod, float dx, float dy)
		{
			constexpr float sensitivity = -0.1f; // TODO: parameter + config file
			float factor = std::pow(2.f, (dx + dy) * sensitivity);
			Vec2f mouse(mousePos_);
			Vec2f before = view_.toWorld().apply(mouse);
			view_.zoom /= factor;
			Vec2f after = view_.toWorld().apply(mouse);
			view_.center += before - after;
		},
		hwnd);

	mfb_set_mouse_move_callback(
		[this](mfb_window*, int x, int y)
		{
			Vec2i newMousePos{x, y};
			if (isMouseDraggingCamera_)
			{
				constexpr float sensitivity = -1.f; // TODO: parameter + config file
				Transform toWorld = view_.toWorld();
				Vec2f drag = toWorld.apply(Vec2f(newMousePos)) - toWorld.apply(Vec2f(mousePos_));
				view_.center += drag * sensitivity;
			}
			mousePos_ = newMousePos;
		},
		hwnd);

	mfb_set_char_input_callback(
		[this](mfb_window*, unsigned int code)
		{
			std::visit([&](auto& ui) { ui(UIEvent::Typed{*this, char32_t(code)}); }, ui_);
		},
		hwnd);
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
			std::visit([&](auto& ui) { ui(UIEvent::Resized{*this, window_.getSize()}); }, ui_);
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
