#pragma once
#include "ModuleTracker.h"
#include "Core/Canvas.h"
#include <array>
#include <cstddef>
#include <optional>

namespace Profiling
{

struct ModuleListView
{
	std::size_t scroll{};
	std::optional<std::size_t> selected;
	std::array<Recti, MAX_MODULE_EVENTS> rows{};
	std::size_t firstRow{};
	std::size_t rowCount{};
	std::size_t visibleRows{};

	std::optional<std::size_t> hitRow(Vec2i point) const
	{
		for (std::size_t i = 0; i < rowCount; ++i)
			if (rows[i].contains(point))
				return firstRow + i;
		return std::nullopt;
	}

	void scrollBy(int lines);
};

void drawModuleList(Canvas canvas, ModuleListView& view, int top);
bool areModuleSymbolsLoaded();
void releaseModuleSymbols();

} // namespace Profiling
