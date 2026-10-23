#pragma once
#include "Profile.h"
#include <cstddef>

namespace Profiling
{

void* trackedMalloc(Counter counter, std::size_t size);
void* trackedRealloc(Counter counter, void* block, std::size_t size);
void trackedFree(Counter counter, void* block);

} // namespace Profiling
