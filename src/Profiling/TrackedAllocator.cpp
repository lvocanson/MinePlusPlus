#include "TrackedAllocator.h"
#include "CounterRegistry.h"
#include <cstdlib>
#include <cstring>

namespace
{

constexpr std::size_t HEADER_SIZE = alignof(std::max_align_t);
static_assert(HEADER_SIZE >= sizeof(std::size_t));

std::byte* headerOf(void* block)
{
	return static_cast<std::byte*>(block) - HEADER_SIZE;
}

std::size_t sizeOf(void* block)
{
	std::size_t size;
	std::memcpy(&size, headerOf(block), sizeof(size));
	return size;
}

void* finish(std::byte* header, std::size_t size)
{
	std::memcpy(header, &size, sizeof(size));
	return header + HEADER_SIZE;
}

} // namespace

// TODO: only the requested bytes are counted, not the CRT heap overhead
void* Profiling::trackedMalloc(Counter counter, std::size_t size)
{
	auto* header = static_cast<std::byte*>(std::malloc(size + HEADER_SIZE));
	if (!header)
		return nullptr;

	adjustCounter(counter, std::ptrdiff_t(size));
	return finish(header, size);
}

void* Profiling::trackedRealloc(Counter counter, void* block, std::size_t size)
{
	if (!block)
		return trackedMalloc(counter, size);

	if (size == 0)
	{
		trackedFree(counter, block);
		return nullptr;
	}

	std::size_t oldSize = sizeOf(block);
	auto* header = static_cast<std::byte*>(std::realloc(headerOf(block), size + HEADER_SIZE));
	if (!header)
		return nullptr;

	adjustCounter(counter, std::ptrdiff_t(size) - std::ptrdiff_t(oldSize));
	return finish(header, size);
}

void Profiling::trackedFree(Counter counter, void* block)
{
	if (!block)
		return;

	adjustCounter(counter, -std::ptrdiff_t(sizeOf(block)));
	std::free(headerOf(block));
}
