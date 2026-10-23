#include "MiniaudioAllocators.h"
#include "TrackedAllocator.h"
#include <cstdint>

namespace
{

Profiling::Counter counterOf(void* user)
{
	return {std::uint8_t(reinterpret_cast<std::uintptr_t>(user))};
}

void* onMalloc(size_t size, void* user)
{
	return Profiling::trackedMalloc(counterOf(user), size);
}

void* onRealloc(void* block, size_t size, void* user)
{
	return Profiling::trackedRealloc(counterOf(user), block, size);
}

void onFree(void* block, void* user)
{
	Profiling::trackedFree(counterOf(user), block);
}

} // namespace

ma_allocation_callbacks Profiling::makeMiniaudioCallbacks(std::string_view counterName)
{
	Counter counter = registerCounter(counterName);
	return {
		reinterpret_cast<void*>(std::uintptr_t(counter.index)),
		onMalloc,
		onRealloc,
		onFree
	};
}
