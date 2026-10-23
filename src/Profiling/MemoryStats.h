#pragma once
#include <algorithm>
#include <cstddef>

namespace Profiling
{

struct MemoryStats
{
	std::size_t privateBytes;
	std::size_t privateWorkingSet;
	std::size_t workingSet;
	std::size_t peakWorkingSet;
};

MemoryStats queryMemoryStats();

constexpr MemoryStats saturatingSub(const MemoryStats& lhs, const MemoryStats& rhs)
{
	auto sub = [](std::size_t a, std::size_t b) { return a > b ? a - b : std::size_t(0); };
	return {
		sub(lhs.privateBytes, rhs.privateBytes),
		sub(lhs.privateWorkingSet, rhs.privateWorkingSet),
		sub(lhs.workingSet, rhs.workingSet),
		lhs.peakWorkingSet
	};
}

constexpr MemoryStats operator+(const MemoryStats& lhs, const MemoryStats& rhs)
{
	return {
		lhs.privateBytes + rhs.privateBytes,
		lhs.privateWorkingSet + rhs.privateWorkingSet,
		lhs.workingSet + rhs.workingSet,
		std::max(lhs.peakWorkingSet, rhs.peakWorkingSet)
	};
}

} // namespace Profiling
