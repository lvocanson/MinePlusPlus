#include "Profiler.h"
#include "MemoryGraph.h"
#include "ModuleList.h"
#include "ProfilerDraw.h"
#include <MiniFB.h>
#include <array>
#include <cmath>
#include <format>
#include <vector>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

namespace
{

namespace Draw = Profiling::Draw;

constexpr int TOGGLE_KEY = VK_F1;
constexpr float POLL_PERIOD = 1.f;
constexpr Vec2u WINDOW_SIZE = {720, 400};

enum class Tab : std::uint8_t
{
	Memory,
	Modules
};

struct Profiler
{
	Profiling::MemoryHistory history;
	Profiling::GraphView view;
	Profiling::ModuleListView modules;
	Tab tab = Tab::Memory;
	std::array<Recti, 2> tabButtons{};
	std::size_t drawnModuleCount = 0;

	Profiling::MemoryStats beforeOpen{};
	Profiling::MemoryStats windowCost{};
	Profiling::MemoryStats symbolsCost{};
	bool measureWindow = false;
	float sinceLastPoll = POLL_PERIOD;
	bool toggleWasDown = false;

	mfb_window* window = nullptr;
	std::vector<std::uint32_t> buffer;
	Vec2i bufferSize{};
	bool dirty = true;
};

Profiler profiler;

Profiling::MemoryStats bufferCost()
{
	std::size_t bytes = profiler.buffer.capacity() * sizeof(std::uint32_t);
	return {bytes, bytes, bytes, 0};
}

// TODO: system allocations the window makes after its first frame (IME, text services) are not measured
Profiling::MemoryStats currentOverhead()
{
	if (!profiler.window)
		return {};
	return profiler.windowCost + bufferCost() + profiler.symbolsCost;
}

void releaseSymbols()
{
	Profiling::releaseModuleSymbols();
	profiler.symbolsCost = {};
}

void releaseWindow()
{
	profiler.windowCost = {};
	profiler.measureWindow = false;
	profiler.window = nullptr;
	std::vector<std::uint32_t>().swap(profiler.buffer);
	profiler.bufferSize = {};
}

void openWindow()
{
	profiler.beforeOpen = Profiling::queryMemoryStats();
	profiler.measureWindow = true;
	profiler.window = mfb_open_ex("Mine++ profiler", WINDOW_SIZE.x, WINDOW_SIZE.y, MFB_WF_RESIZABLE);
	profiler.dirty = true;
	if (!profiler.window)
		return;

	mfb_set_mouse_button_callback(
		[](mfb_window* window, mfb_mouse_button button, mfb_key_mod, bool pressed)
		{
			if (!pressed || button != MFB_MOUSE_LEFT)
				return;

			Vec2i mouse{mfb_get_mouse_x(window), mfb_get_mouse_y(window)};
			for (std::size_t i = 0; i < profiler.tabButtons.size(); ++i)
			{
				if (profiler.tabButtons[i].contains(mouse))
				{
					profiler.tab = Tab(i);
					if (profiler.tab != Tab::Modules)
						releaseSymbols();
					profiler.dirty = true;
					return;
				}
			}

			if (profiler.tab == Tab::Memory)
			{
				if (profiler.view.profilerButton.contains(mouse))
				{
					profiler.view.includeProfiler = !profiler.view.includeProfiler;
					profiler.dirty = true;
				}
				else if (auto series = profiler.view.hitLegend(mouse))
				{
					profiler.view.hidden.flip(*series);
					profiler.dirty = true;
				}
			}
			else if (auto row = profiler.modules.hitRow(mouse))
			{
				profiler.modules.selected = profiler.modules.selected == row ? std::nullopt : row;
				profiler.dirty = true;
			}
		},
		profiler.window);

	mfb_set_mouse_scroll_callback(
		[](mfb_window*, mfb_key_mod, float, float deltaY)
		{
			if (profiler.tab != Tab::Modules)
				return;

			profiler.modules.scrollBy(deltaY > 0.f ? -3 : 3);
			profiler.dirty = true;
		},
		profiler.window);
}

void closeWindow()
{
	if (profiler.window)
	{
		mfb_close(profiler.window);
		[[maybe_unused]] auto _ = mfb_update_events(profiler.window);
	}
	releaseSymbols();
	releaseWindow();
}

bool isFocused(mfb_window* mainWindow)
{
	return (mainWindow && mfb_is_window_active(mainWindow))
	    || (profiler.window && mfb_is_window_active(profiler.window));
}

void handleToggle(mfb_window* mainWindow)
{
	bool down = (GetAsyncKeyState(TOGGLE_KEY) & 0x8000) != 0;
	if (down && !profiler.toggleWasDown && isFocused(mainWindow))
	{
		if (profiler.window)
			closeWindow();
		else
			openWindow();
	}
	profiler.toggleWasDown = down;
}

void poll(float dt)
{
	profiler.sinceLastPoll += dt;
	if (profiler.sinceLastPoll < POLL_PERIOD)
		return;

	profiler.sinceLastPoll = std::fmod(profiler.sinceLastPoll, POLL_PERIOD);
	Profiling::Sample sample{Profiling::queryMemoryStats(), currentOverhead(), {}};
	for (std::size_t i = 0; i < Profiling::getCounterCount(); ++i)
		sample.counters[i] = Profiling::sampleCounter(i);
	profiler.history.push(sample);
	profiler.dirty = true;
}

// TODO: the overhead leaves out what the system allocates for the window itself
void updateTitle()
{
	std::array<char, 128> title{};
	std::size_t overhead = currentOverhead().privateBytes;
	double peak = profiler.history.empty() ? 0.0 : double(profiler.history.latest().system.peakWorkingSet) / (1024.0 * 1024.0);
	std::format_to_n(
		title.data(),
		title.size() - 1,
		"Mine++ profiler | peak WS {:.1f} MB | profiler overhead {} KB",
		peak,
		overhead / 1024);
	mfb_set_title(profiler.window, title.data());
}

void draw(Canvas canvas)
{
	using namespace Profiling::Draw;
	fill(canvas, BACKGROUND_COLOR);

	Draw::Label label;
	Vec2i cursor{MARGIN, MARGIN};
	profiler.tabButtons[0] = drawButton(canvas, cursor, "Memory", profiler.tab == Tab::Memory);
	cursor.x += profiler.tabButtons[0].size.x + BUTTON_GAP;
	profiler.drawnModuleCount = Profiling::getModuleEventCount();
	profiler.tabButtons[1] = drawButton(canvas, cursor, label("Modules ({})", profiler.drawnModuleCount), profiler.tab == Tab::Modules);

	int contentTop = MARGIN + profiler.tabButtons[0].size.y + BUTTON_GAP;
	fillRect(canvas, {{MARGIN, contentTop - BUTTON_GAP / 2}, {canvas.size.x - MARGIN * 2, 1}}, GRID_COLOR);

	if (profiler.tab == Tab::Memory)
	{
		Profiling::drawMemoryGraph(canvas, profiler.history, profiler.view, contentTop);
		return;
	}

	bool symbolsLoaded = Profiling::areModuleSymbolsLoaded();
	Profiling::MemoryStats before = symbolsLoaded ? Profiling::MemoryStats{} : Profiling::queryMemoryStats();
	Profiling::drawModuleList(canvas, profiler.modules, contentTop);
	if (!symbolsLoaded && Profiling::areModuleSymbolsLoaded())
		profiler.symbolsCost = Profiling::saturatingSub(Profiling::queryMemoryStats(), before);
}

void present()
{
	if (!profiler.window)
		return;

	Vec2u size{};
	mfb_get_window_size(profiler.window, &size.x, &size.y);
	if (Vec2i(size) != profiler.bufferSize)
	{
		profiler.bufferSize = Vec2i(size);
		profiler.buffer.resize(std::size_t(size.x) * std::size_t(size.y));
		profiler.buffer.shrink_to_fit();
		profiler.dirty = true;
	}

	if (profiler.tab == Tab::Modules && Profiling::getModuleEventCount() != profiler.drawnModuleCount)
		profiler.dirty = true;

	if (profiler.dirty && !profiler.buffer.empty())
	{
		draw({profiler.buffer.data(), profiler.bufferSize});
		updateTitle();
		profiler.dirty = false;
	}

	mfb_update_state state = profiler.buffer.empty()
	                         ? mfb_update_events(profiler.window)
	                         : mfb_update_ex(profiler.window, profiler.buffer.data(), size.x, size.y);
	if (state == MFB_STATE_EXIT)
	{
		releaseSymbols();
		releaseWindow();
	}
	else if (profiler.measureWindow && !profiler.buffer.empty())
	{
		profiler.measureWindow = false;
		Profiling::MemoryStats opened = Profiling::saturatingSub(Profiling::queryMemoryStats(), profiler.beforeOpen);
		profiler.windowCost = Profiling::saturatingSub(opened, bufferCost());
		profiler.dirty = true;
	}
}

} // namespace

void Profiling::update(mfb_window* mainWindow, float dt)
{
	handleToggle(mainWindow);
	poll(dt);
	present();
}

void Profiling::shutdown()
{
	closeWindow();
}
