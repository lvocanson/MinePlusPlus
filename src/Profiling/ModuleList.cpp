#include "ModuleList.h"
#include "ProfilerDraw.h"
#include <algorithm>
#include <cctype>
#include <cmath>
#include <span>
#include <string_view>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <DbgHelp.h>
#include <Psapi.h>

namespace
{

using namespace Profiling;
using namespace Profiling::Draw;

using Line = std::array<char, 128>;

struct Symbols
{
	bool ready = false;
	std::array<Line, MAX_MODULE_EVENTS> loadedBy{};
	std::size_t resolvedCount = 0;
	std::optional<std::size_t> stackOwner;
	std::array<Line, MAX_MODULE_FRAMES> stack{};
	std::size_t stackSize = 0;
	std::size_t hiddenLoaderFrames = 0;
};

Symbols symbols;

constexpr int COLUMN_NAME = 84;
constexpr int COLUMN_SIZE = 270;
constexpr int COLUMN_LOADED_BY = 360;

void ensureSymbols()
{
	if (symbols.ready)
		return;

	SymSetOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS | SYMOPT_NO_PROMPTS | SYMOPT_FAIL_CRITICAL_ERRORS);
	symbols.ready = SymInitialize(GetCurrentProcess(), nullptr, TRUE) != FALSE;
}

std::string_view moduleNameOf(const void* address, std::span<char> out)
{
	HMODULE module{};
	DWORD flags = GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT;
	if (!GetModuleHandleExA(flags, static_cast<LPCSTR>(address), &module)
	    || !GetModuleBaseNameA(GetCurrentProcess(), module, out.data(), DWORD(out.size())))
		return "?";
	return out.data();
}

bool isLoaderModule(std::string_view name)
{
	auto equals = [name](std::string_view other)
	{
		return name.size() == other.size()
		    && std::ranges::equal(name, other, [](char a, char b) { return std::tolower(a) == std::tolower(b); });
	};
	return equals("ntdll.dll") || equals("kernelbase.dll") || equals("kernel32.dll");
}

void describe(const void* address, Line& out)
{
	std::array<char, 64> module{};
	std::string_view moduleName = moduleNameOf(address, module);

	alignas(SYMBOL_INFO) std::array<char, sizeof(SYMBOL_INFO) + 256> buffer{};
	auto* symbol = reinterpret_cast<SYMBOL_INFO*>(buffer.data());
	symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
	symbol->MaxNameLen = 255;

	DWORD64 displacement = 0;
	auto result = symbols.ready && SymFromAddr(GetCurrentProcess(), DWORD64(address), &displacement, symbol)
	              ? std::format_to_n(out.data(), out.size() - 1, "{}!{}+0x{:X}", moduleName, symbol->Name, displacement)
	              : std::format_to_n(out.data(), out.size() - 1, "{}!0x{:X}", moduleName, std::uintptr_t(address));
	*result.out = '\0';
}

std::size_t firstRequesterFrame(const ModuleEvent& event)
{
	std::array<char, 64> module{};
	std::size_t frame = 0;
	while (frame < event.frameCount && isLoaderModule(moduleNameOf(event.frames[frame], module)))
		++frame;
	return frame;
}

void resolveLoadedBy(std::size_t index)
{
	const ModuleEvent& event = getModuleEvent(index);
	Line& out = symbols.loadedBy[index];
	if (event.preloaded)
	{
		*std::format_to_n(out.data(), out.size() - 1, "process startup").out = '\0';
		return;
	}

	std::size_t frame = firstRequesterFrame(event);
	if (frame == event.frameCount)
	{
		*std::format_to_n(out.data(), out.size() - 1, "loader").out = '\0';
		return;
	}

	describe(event.frames[frame], out);
}

void resolveStack(std::size_t index)
{
	const ModuleEvent& event = getModuleEvent(index);
	symbols.stackOwner = index;
	symbols.hiddenLoaderFrames = firstRequesterFrame(event);
	symbols.stackSize = event.frameCount - symbols.hiddenLoaderFrames;
	for (std::size_t i = 0; i < symbols.stackSize; ++i)
		describe(event.frames[symbols.hiddenLoaderFrames + i], symbols.stack[i]);
}

std::string_view formatSize(Label& label, std::uint32_t bytes)
{
	return bytes >= 1024 * 1024
	       ? label("{:.1f} MB", double(bytes) / (1024.0 * 1024.0))
	       : label("{} KB", bytes / 1024);
}

void drawRow(Canvas canvas, const ModuleEvent& event, std::size_t index, Recti row, bool selected)
{
	if (selected)
		fillRect(canvas, row, ACTIVE_BUTTON_COLOR);

	Color color = event.isLoaded() ? TEXT_COLOR : MUTED_COLOR;
	float baseline = float(row.position.y + row.size.y / 2 + TEXT_BASELINE_OFFSET);
	float x = float(row.position.x);

	Label label;
	std::string_view time = event.preloaded ? "start" : label("{:.2f}s", event.loadedAt);
	drawText(canvas, {x, baseline}, time, color);
	drawText(canvas, {x + COLUMN_NAME, baseline}, fitText(event.getName(), float(COLUMN_SIZE - COLUMN_NAME - 8)), color);

	std::string_view size = formatSize(label, event.imageSize);
	drawText(canvas, {x + COLUMN_LOADED_BY - 16 - textWidth(size), baseline}, size, color);

	std::string_view loadedBy = symbols.loadedBy[index].data();
	float available = float(row.size.x - COLUMN_LOADED_BY);
	drawText(canvas, {x + COLUMN_LOADED_BY, baseline}, fitText(loadedBy, available), event.isLoaded() ? MUTED_COLOR : GRID_COLOR);
}

} // namespace

void Profiling::ModuleListView::scrollBy(int lines)
{
	std::size_t count = getModuleEventCount();
	std::size_t maxScroll = count > visibleRows ? count - visibleRows : 0;
	long long next = (long long)scroll + lines;
	scroll = std::size_t(std::clamp<long long>(next, 0, (long long)maxScroll));
}

void Profiling::drawModuleList(Canvas canvas, ModuleListView& view, int top)
{
	ensureSymbols();
	std::size_t count = getModuleEventCount();
	for (; symbols.resolvedCount < count; ++symbols.resolvedCount)
		resolveLoadedBy(symbols.resolvedCount);

	int rowHeight = lineHeight() + 2;
	int width = canvas.size.x - MARGIN * 2;
	int listBottom = view.selected ? top + (canvas.size.y - top) / 2 : canvas.size.y - MARGIN;

	Label label;
	float headerBaseline = float(top + rowHeight / 2 + TEXT_BASELINE_OFFSET);
	drawText(canvas, {float(MARGIN), headerBaseline}, "Time", MUTED_COLOR);
	drawText(canvas, {float(MARGIN + COLUMN_NAME), headerBaseline}, "Module", MUTED_COLOR);
	drawText(canvas, {float(MARGIN + COLUMN_LOADED_BY - 16) - textWidth("Image"), headerBaseline}, "Image", MUTED_COLOR);
	drawText(canvas, {float(MARGIN + COLUMN_LOADED_BY), headerBaseline}, "Loaded by", MUTED_COLOR);
	fillRect(canvas, {{MARGIN, top + rowHeight}, {width, 1}}, GRID_COLOR);

	int listTop = top + rowHeight + 2;
	view.visibleRows = std::size_t(std::max(0, (listBottom - listTop) / rowHeight));
	view.scrollBy(0);
	view.firstRow = view.scroll;
	view.rowCount = std::min(view.visibleRows, count - std::min(count, view.firstRow));

	for (std::size_t i = 0; i < view.rowCount; ++i)
	{
		std::size_t index = view.firstRow + i;
		Recti row{{MARGIN, listTop + int(i) * rowHeight}, {width, rowHeight}};
		view.rows[i] = row;
		drawRow(canvas, getModuleEvent(index), index, row, view.selected == index);
	}

	if (!view.selected || *view.selected >= count)
		return;

	if (symbols.stackOwner != view.selected)
		resolveStack(*view.selected);

	int detailsTop = listBottom + MARGIN / 2;
	fillRect(canvas, {{MARGIN, detailsTop}, {width, 1}}, GRID_COLOR);
	const ModuleEvent& event = getModuleEvent(*view.selected);
	float baseline = float(detailsTop + rowHeight / 2 + TEXT_BASELINE_OFFSET);
	std::string_view title = event.preloaded
	                         ? label("{} load stack", event.getName())
	                         : label("{} load stack ({} loader frames hidden)", event.getName(), symbols.hiddenLoaderFrames);
	drawText(canvas, {float(MARGIN), baseline}, title, TEXT_COLOR);

	if (event.preloaded)
	{
		drawText(canvas, {float(MARGIN), baseline + float(rowHeight)}, "Loaded by the system before the profiler started", MUTED_COLOR);
		return;
	}

	for (std::size_t i = 0; i < symbols.stackSize; ++i)
	{
		float y = baseline + float(rowHeight) * float(i + 1);
		if (y > float(canvas.size.y - MARGIN))
			break;
		drawText(canvas, {float(MARGIN), y}, fitText(symbols.stack[i].data(), float(width)), MUTED_COLOR);
	}
}

bool Profiling::areModuleSymbolsLoaded()
{
	return symbols.ready;
}

void Profiling::releaseModuleSymbols()
{
	if (symbols.ready)
		SymCleanup(GetCurrentProcess());
	symbols = {};
}
