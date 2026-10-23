#include "ModuleTracker.h"
#include <algorithm>
#include <chrono>
#include <cstring>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <winternl.h>
#include <Psapi.h>

namespace
{

using Profiling::ModuleEvent;

struct LdrDllNotificationData
{
	ULONG flags;
	PCUNICODE_STRING fullDllName;
	PCUNICODE_STRING baseDllName;
	PVOID dllBase;
	ULONG sizeOfImage;
};

using LdrDllNotificationFunction = VOID(CALLBACK*)(ULONG reason, const LdrDllNotificationData* data, PVOID context);
using LdrRegisterDllNotification = NTSTATUS(NTAPI*)(ULONG flags, LdrDllNotificationFunction function, PVOID context, PVOID* cookie);

constexpr ULONG REASON_LOADED = 1;
constexpr ULONG REASON_UNLOADED = 2;

std::array<ModuleEvent, Profiling::MAX_MODULE_EVENTS> events{};
std::atomic<std::size_t> eventCount = 0;
std::chrono::steady_clock::time_point startTime;

float secondsSinceStart()
{
	return std::chrono::duration<float>(std::chrono::steady_clock::now() - startTime).count();
}

bool isRecorded(std::uintptr_t base)
{
	for (std::size_t i = 0, count = eventCount.load(std::memory_order_acquire); i < count; ++i)
		if (events[i].base == base && events[i].isLoaded())
			return true;
	return false;
}

ModuleEvent* beginRecord(std::uintptr_t base, std::uint32_t imageSize, bool preloaded)
{
	std::size_t index = eventCount.load(std::memory_order_relaxed);
	if (index >= events.size())
		return nullptr;

	ModuleEvent& event = events[index];
	event.base = base;
	event.imageSize = imageSize;
	event.preloaded = preloaded;
	event.loadedAt = preloaded ? 0.f : secondsSinceStart();
	event.unloadedAt.store(-1.f, std::memory_order_relaxed);
	event.frameCount = 0;
	return &event;
}

void endRecord()
{
	eventCount.fetch_add(1, std::memory_order_release);
}

template <class Char>
void copyName(ModuleEvent& event, const Char* name, std::size_t length)
{
	std::size_t count = std::min(length, event.name.size() - 1);
	for (std::size_t i = 0; i < count; ++i)
		event.name[i] = name[i] > 0 && name[i] < 0x80 ? char(name[i]) : '?';
	event.name[count] = '\0';
}

VOID CALLBACK onDllNotification(ULONG reason, const LdrDllNotificationData* data, PVOID)
{
	auto base = reinterpret_cast<std::uintptr_t>(data->dllBase);
	if (reason == REASON_LOADED)
	{
		ModuleEvent* event = beginRecord(base, data->sizeOfImage, false);
		if (!event)
			return;

		copyName(*event, data->baseDllName->Buffer, data->baseDllName->Length / sizeof(WCHAR));
		event->frameCount = CaptureStackBackTrace(1, DWORD(event->frames.size()), event->frames.data(), nullptr);
		endRecord();
	}
	else if (reason == REASON_UNLOADED)
	{
		for (std::size_t i = 0, count = eventCount.load(std::memory_order_acquire); i < count; ++i)
			if (events[i].base == base && events[i].isLoaded())
				events[i].unloadedAt.store(secondsSinceStart(), std::memory_order_relaxed);
	}
}

// TODO: a module loaded by another thread while the snapshot runs may be recorded twice
void recordAlreadyLoadedModules()
{
	std::array<HMODULE, 256> modules{};
	DWORD needed = 0;
	HANDLE process = GetCurrentProcess();
	if (!EnumProcessModules(process, modules.data(), DWORD(sizeof(modules)), &needed))
		return;

	std::size_t count = std::min<std::size_t>(needed / sizeof(HMODULE), modules.size());
	for (std::size_t i = 0; i < count; ++i)
	{
		auto base = reinterpret_cast<std::uintptr_t>(modules[i]);
		MODULEINFO info{};
		std::array<char, 48> name{};
		if (isRecorded(base) || !GetModuleInformation(process, modules[i], &info, sizeof(info))
		    || !GetModuleBaseNameA(process, modules[i], name.data(), DWORD(name.size())))
			continue;

		if (ModuleEvent* event = beginRecord(base, info.SizeOfImage, true))
		{
			copyName(*event, name.data(), std::strlen(name.data()));
			endRecord();
		}
	}
}

bool startTracking()
{
	startTime = std::chrono::steady_clock::now();

	auto registerNotification = reinterpret_cast<LdrRegisterDllNotification>(
		GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "LdrRegisterDllNotification"));
	PVOID cookie = nullptr;
	if (registerNotification)
		registerNotification(0, onDllNotification, nullptr, &cookie);

	recordAlreadyLoadedModules();
	return cookie != nullptr;
}

[[maybe_unused]] const bool trackingStarted = startTracking();

} // namespace

std::size_t Profiling::getModuleEventCount()
{
	return eventCount.load(std::memory_order_acquire);
}

const ModuleEvent& Profiling::getModuleEvent(std::size_t index)
{
	return events[index];
}
