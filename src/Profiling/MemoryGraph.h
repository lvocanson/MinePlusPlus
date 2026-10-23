#pragma once
#include "CounterRegistry.h"
#include "MemoryStats.h"
#include "Core/Canvas.h"
#include <array>
#include <bitset>
#include <cstddef>
#include <optional>

namespace Profiling
{

struct Sample
{
	MemoryStats system;
	MemoryStats profilerOverhead;
	std::array<std::size_t, MAX_COUNTERS> counters;
};

class MemoryHistory
{
public:

	static constexpr std::size_t CAPACITY = 300;

	void push(const Sample& sample)
	{
		samples_[next_] = sample;
		next_ = (next_ + 1) % CAPACITY;
		count_ = count_ < CAPACITY ? count_ + 1 : CAPACITY;
	}

	std::size_t size() const { return count_; }
	bool empty() const { return count_ == 0; }

	const Sample& operator[](std::size_t oldestFirst) const
	{
		return samples_[(next_ + CAPACITY - count_ + oldestFirst) % CAPACITY];
	}

	const Sample& latest() const { return (*this)[count_ - 1]; }

private:

	std::array<Sample, CAPACITY> samples_{};
	std::size_t next_{};
	std::size_t count_{};
};

constexpr std::size_t SYSTEM_SERIES = 3;
constexpr std::size_t MAX_SERIES = SYSTEM_SERIES + MAX_COUNTERS;

struct GraphView
{
	std::bitset<MAX_SERIES> hidden;
	std::array<Recti, MAX_SERIES> legendButtons{};
	std::size_t seriesCount{};
	bool includeProfiler = false;
	Recti profilerButton{};

	std::optional<std::size_t> hitLegend(Vec2i point) const
	{
		for (std::size_t i = 0; i < seriesCount; ++i)
			if (legendButtons[i].contains(point))
				return i;
		return std::nullopt;
	}
};

void drawMemoryGraph(Canvas canvas, const MemoryHistory& history, GraphView& view, int top);

} // namespace Profiling
