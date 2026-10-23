#include "MemoryStats.h"
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <Psapi.h>

Profiling::MemoryStats Profiling::queryMemoryStats()
{
	PROCESS_MEMORY_COUNTERS_EX2 pmc{};
	if (!GetProcessMemoryInfo(GetCurrentProcess(), reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&pmc), sizeof(pmc)))
		return {};
	return {pmc.PrivateUsage, pmc.PrivateWorkingSetSize, pmc.WorkingSetSize, pmc.PeakWorkingSetSize};
}
