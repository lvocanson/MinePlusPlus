#pragma once
#include <miniaudio.h>
#include <string_view>

namespace Profiling
{

ma_allocation_callbacks makeMiniaudioCallbacks(std::string_view counterName);

} // namespace Profiling
