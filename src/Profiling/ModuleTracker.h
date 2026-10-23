#pragma once
#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <string_view>

namespace Profiling
{

constexpr std::size_t MAX_MODULE_EVENTS = 128;
constexpr std::size_t MAX_MODULE_FRAMES = 32;

struct ModuleEvent
{
	std::array<char, 48> name;
	std::uintptr_t base;
	std::uint32_t imageSize;
	bool preloaded;
	float loadedAt;
	std::atomic<float> unloadedAt;
	std::uint16_t frameCount;
	std::array<void*, MAX_MODULE_FRAMES> frames;

	std::string_view getName() const { return name.data(); }
	bool isLoaded() const { return unloadedAt.load(std::memory_order_relaxed) < 0.f; }
};

std::size_t getModuleEventCount();
const ModuleEvent& getModuleEvent(std::size_t index);

} // namespace Profiling
