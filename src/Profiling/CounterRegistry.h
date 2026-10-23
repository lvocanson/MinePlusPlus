#pragma once
#include "Profile.h"
#include <cstddef>
#include <string_view>

namespace Profiling
{

constexpr std::size_t MAX_COUNTERS = 12;

void adjustCounter(Counter counter, std::ptrdiff_t delta);

std::size_t getCounterCount();
std::string_view getCounterName(std::size_t index);
std::size_t sampleCounter(std::size_t index);

} // namespace Profiling
