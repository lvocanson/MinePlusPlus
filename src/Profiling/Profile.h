#pragma once

#ifdef MPP_PROFILING

#include <cstddef>
#include <cstdint>
#include <string_view>

namespace Profiling
{

struct Counter
{
	std::uint8_t index;
};

Counter registerCounter(std::string_view name);
void setCounter(Counter counter, std::size_t bytes);
void reportCounterPeak(Counter counter, std::size_t bytes);

} // namespace Profiling

#define MPP_PROFILING_COUNTER_CALL(function, name, bytes) \
	do \
	{ \
		static const ::Profiling::Counter mppProfilingCounter = ::Profiling::registerCounter(name); \
		::Profiling::function(mppProfilingCounter, (bytes)); \
	} while (false)

#define MPP_PROFILE_MEMORY(name, bytes) MPP_PROFILING_COUNTER_CALL(setCounter, name, bytes)
#define MPP_PROFILE_MEMORY_PEAK(name, bytes) MPP_PROFILING_COUNTER_CALL(reportCounterPeak, name, bytes)

#else

#define MPP_PROFILE_MEMORY(name, bytes) ((void)0)
#define MPP_PROFILE_MEMORY_PEAK(name, bytes) ((void)0)

#endif // MPP_PROFILING
