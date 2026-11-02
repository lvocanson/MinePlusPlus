#include "VirtualRegion.h"
#include <limits>
#include <optional>
#include <utility>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#else
#include <sys/mman.h>
#include <unistd.h>
#endif

namespace
{

std::size_t getPageSize()
{
#ifdef _WIN32
	static const std::size_t pageSize = []
	{
		SYSTEM_INFO info;
		GetSystemInfo(&info);
		return std::size_t(info.dwPageSize);
	}();
#else
	static const std::size_t pageSize = std::size_t(sysconf(_SC_PAGESIZE));
#endif
	return pageSize;
}

std::optional<std::size_t> roundUpToPage(std::size_t bytes)
{
	std::size_t mask = getPageSize() - 1;
	if (bytes > std::numeric_limits<std::size_t>::max() - mask)
		return std::nullopt;

	return (bytes + mask) & ~mask;
}

} // namespace

VirtualRegion::VirtualRegion(VirtualRegion&& other) noexcept
	: base_{std::exchange(other.base_, nullptr)}
	, reserved_{std::exchange(other.reserved_, 0)}
	, committed_{std::exchange(other.committed_, 0)}
{}

VirtualRegion& VirtualRegion::operator=(VirtualRegion&& other) noexcept
{
	if (this != &other)
	{
		release();
		base_ = std::exchange(other.base_, nullptr);
		reserved_ = std::exchange(other.reserved_, 0);
		committed_ = std::exchange(other.committed_, 0);
	}
	return *this;
}

VirtualRegion::~VirtualRegion()
{
	release();
}

bool VirtualRegion::reserve(std::size_t maxBytes)
{
	release();

	std::optional<std::size_t> bytes = roundUpToPage(maxBytes);
	if (!bytes)
		return false;
	if (*bytes == 0)
		return true;

#ifdef _WIN32
	void* base = VirtualAlloc(nullptr, *bytes, MEM_RESERVE, PAGE_NOACCESS);
	if (!base)
		return false;
#else
	void* base = mmap(nullptr, *bytes, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE, -1, 0);
	if (base == MAP_FAILED)
		return false;
#endif

	base_ = static_cast<std::byte*>(base);
	reserved_ = *bytes;
	return true;
}

bool VirtualRegion::commitTo(std::size_t bytes)
{
	std::optional<std::size_t> target = roundUpToPage(bytes);
	if (!target || *target > reserved_)
		return false;
	if (*target <= committed_)
		return true;

	std::byte* begin = base_ + committed_;
	std::size_t size = *target - committed_;
#ifdef _WIN32
	if (!VirtualAlloc(begin, size, MEM_COMMIT, PAGE_READWRITE))
		return false;
#else
	if (mprotect(begin, size, PROT_READ | PROT_WRITE) != 0)
		return false;
#endif

	committed_ = *target;
	return true;
}

void VirtualRegion::decommitTo(std::size_t bytes)
{
	std::optional<std::size_t> target = roundUpToPage(bytes);
	if (!target || *target >= committed_)
		return;

	std::byte* begin = base_ + *target;
	std::size_t size = committed_ - *target;
#ifdef _WIN32
	VirtualFree(begin, size, MEM_DECOMMIT);
#else
	madvise(begin, size, MADV_DONTNEED);
	mprotect(begin, size, PROT_NONE);
#endif

	committed_ = *target;
}

void VirtualRegion::release()
{
	if (!base_)
		return;

#ifdef _WIN32
	VirtualFree(base_, 0, MEM_RELEASE);
#else
	munmap(base_, reserved_);
#endif

	base_ = nullptr;
	reserved_ = 0;
	committed_ = 0;
}
