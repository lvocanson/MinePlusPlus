#include "Profile.h"
#include "CounterRegistry.h"
#include <array>
#include <atomic>
#include <cassert>

namespace
{

constexpr std::uint8_t INVALID_INDEX = 0xFF;

struct CounterData
{
	std::string_view name;
	std::atomic<std::size_t> current;
	std::atomic<std::size_t> intervalPeak;
};

// TODO: counters must be registered from the main thread, only their values are thread safe
std::array<CounterData, Profiling::MAX_COUNTERS> counters{};
std::atomic<std::size_t> counterCount = 0;

CounterData* find(Profiling::Counter counter)
{
	return counter.index < counterCount.load(std::memory_order_acquire) ? &counters[counter.index] : nullptr;
}

void raisePeak(CounterData& data, std::size_t bytes)
{
	std::size_t peak = data.intervalPeak.load(std::memory_order_relaxed);
	while (peak < bytes && !data.intervalPeak.compare_exchange_weak(peak, bytes, std::memory_order_relaxed))
	{
	}
}

} // namespace

Profiling::Counter Profiling::registerCounter(std::string_view name)
{
	std::size_t count = counterCount.load(std::memory_order_acquire);
	for (std::size_t i = 0; i < count; ++i)
		if (counters[i].name == name)
			return {std::uint8_t(i)};

	assert(count < MAX_COUNTERS && "Too many profiling counters");
	if (count >= MAX_COUNTERS)
		return {INVALID_INDEX};

	counters[count].name = name;
	counterCount.store(count + 1, std::memory_order_release);
	return {std::uint8_t(count)};
}

void Profiling::setCounter(Counter counter, std::size_t bytes)
{
	if (CounterData* data = find(counter))
	{
		data->current.store(bytes, std::memory_order_relaxed);
		raisePeak(*data, bytes);
	}
}

void Profiling::reportCounterPeak(Counter counter, std::size_t bytes)
{
	if (CounterData* data = find(counter))
		raisePeak(*data, bytes);
}

void Profiling::adjustCounter(Counter counter, std::ptrdiff_t delta)
{
	if (CounterData* data = find(counter))
	{
		std::size_t bytes = delta >= 0
		                    ? data->current.fetch_add(std::size_t(delta), std::memory_order_relaxed) + std::size_t(delta)
		                    : data->current.fetch_sub(std::size_t(-delta), std::memory_order_relaxed) - std::size_t(-delta);
		raisePeak(*data, bytes);
	}
}

std::size_t Profiling::getCounterCount()
{
	return counterCount.load(std::memory_order_acquire);
}

std::string_view Profiling::getCounterName(std::size_t index)
{
	return counters[index].name;
}

std::size_t Profiling::sampleCounter(std::size_t index)
{
	CounterData& data = counters[index];
	return data.intervalPeak.exchange(data.current.load(std::memory_order_relaxed), std::memory_order_relaxed);
}
