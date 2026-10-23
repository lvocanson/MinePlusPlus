#include "MemoryGraph.h"
#include "ProfilerDraw.h"
#include <algorithm>
#include <cmath>
#include <string_view>

namespace
{

using namespace Profiling;
using namespace Profiling::Draw;

constexpr int AXIS_WIDTH = 84;
constexpr int GRID_LINES = 5;
constexpr int SWATCH_SIZE = 12;

constexpr std::array<std::string_view, SYSTEM_SERIES> SYSTEM_NAMES{"Private", "Private WS", "WS"};

constexpr std::array<Color, MAX_SERIES> SERIES_COLORS
{
	Color{0xF5, 0x9E, 0x0B},
	Color{0x22, 0xC5, 0x5E},
	Color{0x3B, 0x82, 0xF6},
	Color{0xEC, 0x48, 0x99},
	Color{0x06, 0xB6, 0xD4},
	Color{0xA8, 0x55, 0xF7},
	Color{0xEF, 0x44, 0x44},
	Color{0x84, 0xCC, 0x16},
	Color{0xEA, 0xB3, 0x08},
	Color{0x14, 0xB8, 0xA6},
	Color{0xF4, 0x3F, 0x5E},
	Color{0x81, 0x8C, 0xF8},
	Color{0xFB, 0x92, 0x3C},
	Color{0x34, 0xD3, 0x99},
	Color{0xE8, 0x79, 0xF9},
};

struct Unit
{
	std::string_view suffix;
	double divisor;
};

std::string_view seriesName(std::size_t series)
{
	return series < SYSTEM_SERIES ? SYSTEM_NAMES[series] : getCounterName(series - SYSTEM_SERIES);
}

std::size_t seriesValue(const Sample& sample, std::size_t series, const GraphView& view)
{
	MemoryStats system = view.includeProfiler ? sample.system : saturatingSub(sample.system, sample.profilerOverhead);
	switch (series)
	{
	case 0: return system.privateBytes;
	case 1: return system.privateWorkingSet;
	case 2: return system.workingSet;
	default: return sample.counters[series - SYSTEM_SERIES];
	}
}

Unit unitFor(double bytes)
{
	if (bytes < 1024.0)
		return {"B", 1.0};
	if (bytes < 1024.0 * 1024.0)
		return {"KB", 1024.0};
	return {"MB", 1024.0 * 1024.0};
}

std::string_view formatBytes(Label& label, std::size_t bytes)
{
	Unit unit = unitFor(double(bytes));
	if (unit.divisor == 1.0)
		return label("{} B", bytes);
	return label("{:.1f} {}", double(bytes) / unit.divisor, unit.suffix);
}

double niceStep(double range)
{
	double raw = range / GRID_LINES;
	double magnitude = std::pow(10.0, std::floor(std::log10(raw)));
	for (double multiple : {1.0, 2.0, 5.0})
		if (raw <= multiple * magnitude)
			return multiple * magnitude;
	return 10.0 * magnitude;
}

int drawLegend(Canvas canvas, const MemoryHistory& history, GraphView& view, int top)
{
	int height = lineHeight() + BUTTON_PADDING;
	Vec2i cursor{MARGIN, top};

	for (std::size_t series = 0; series < view.seriesCount; ++series)
	{
		Label valueLabel, textLabel;
		std::size_t value = history.empty() ? 0 : seriesValue(history.latest(), series, view);
		std::string_view text = textLabel("{} {}", seriesName(series), formatBytes(valueLabel, value));

		int width = BUTTON_PADDING * 3 + SWATCH_SIZE + int(std::ceil(textWidth(text)));
		if (cursor.x > MARGIN && cursor.x + width > canvas.size.x - MARGIN)
			cursor = {MARGIN, cursor.y + height + BUTTON_GAP / 2};

		Recti button{cursor, {width, height}};
		view.legendButtons[series] = button;

		bool hidden = view.hidden[series];
		Color color = SERIES_COLORS[series];
		fillRect(canvas, button, BUTTON_COLOR);

		Recti swatch{{button.position.x + BUTTON_PADDING, button.position.y + (height - SWATCH_SIZE) / 2}, {SWATCH_SIZE, SWATCH_SIZE}};
		if (hidden)
			strokeRect(canvas, {swatch.position + Vec2i{1, 1}, swatch.size - Vec2i{2, 2}}, 1, MUTED_COLOR);
		else
			fillRect(canvas, swatch, color);

		Vec2f baseline{
			float(swatch.position.x + SWATCH_SIZE + BUTTON_PADDING),
			float(button.position.y + height / 2 + TEXT_BASELINE_OFFSET)};
		drawText(canvas, baseline, text, hidden ? MUTED_COLOR : color);

		cursor.x += width + BUTTON_GAP;
	}

	Label valueLabel, textLabel;
	std::size_t overhead = history.empty() ? 0 : history.latest().profilerOverhead.privateBytes;
	std::string_view text = view.includeProfiler
	                        ? std::string_view("Profiler included")
	                        : textLabel("Profiler excluded (-{})", formatBytes(valueLabel, overhead));
	int width = BUTTON_PADDING * 2 + int(std::ceil(textWidth(text)));
	if (cursor.x > MARGIN && cursor.x + width > canvas.size.x - MARGIN)
		cursor = {MARGIN, cursor.y + height + BUTTON_GAP / 2};
	view.profilerButton = drawButton(canvas, cursor, text, !view.includeProfiler);

	return cursor.y + height;
}

} // namespace

void Profiling::drawMemoryGraph(Canvas canvas, const MemoryHistory& history, GraphView& view, int top)
{
	view.seriesCount = SYSTEM_SERIES + getCounterCount();
	int legendBottom = drawLegend(canvas, history, view, top);

	Recti plot{
		{AXIS_WIDTH, legendBottom + MARGIN * 2},
		{canvas.size.x - AXIS_WIDTH - MARGIN, canvas.size.y - legendBottom - MARGIN * 3}};
	if (plot.size.x <= 1 || plot.size.y <= 1 || history.empty())
		return;

	std::size_t maxBytes = 0;
	for (std::size_t i = 0; i < history.size(); ++i)
		for (std::size_t series = 0; series < view.seriesCount; ++series)
			if (!view.hidden[series])
				maxBytes = std::max(maxBytes, seriesValue(history[i], series, view));

	Unit unit = unitFor(double(maxBytes));
	double maxValue = double(maxBytes) / unit.divisor;
	double step = niceStep(std::max(maxValue, 1.0));
	double axisTop = std::max(std::ceil(maxValue / step) * step, step);

	auto toY = [&](double value)
	{
		return plot.position.y + plot.size.y - int(std::lround(value / axisTop * plot.size.y));
	};

	Label label;
	for (double value = 0.0; value <= axisTop + step * 0.5; value += step)
	{
		int y = toY(value);
		fillRect(canvas, {{plot.position.x, y}, {plot.size.x, 1}}, GRID_COLOR);
		std::string_view text = label("{:g} {}", value, unit.suffix);
		drawText(canvas, {float(plot.position.x - 8) - textWidth(text), float(y + TEXT_BASELINE_OFFSET)}, text, TEXT_COLOR);
	}

	auto toX = [&](std::size_t sample)
	{
		std::size_t slot = MemoryHistory::CAPACITY - history.size() + sample;
		return plot.position.x + int(slot * std::size_t(plot.size.x - 1) / (MemoryHistory::CAPACITY - 1));
	};

	for (std::size_t series = 0; series < view.seriesCount; ++series)
	{
		if (view.hidden[series])
			continue;

		auto pointAt = [&](std::size_t i)
		{
			return Vec2i{toX(i), toY(double(seriesValue(history[i], series, view)) / unit.divisor)};
		};

		Vec2i previous = pointAt(0);
		for (std::size_t i = 0; i < history.size(); ++i)
		{
			Vec2i point = pointAt(i);
			drawLine(canvas, previous, point, SERIES_COLORS[series]);
			previous = point;
		}
	}
}
