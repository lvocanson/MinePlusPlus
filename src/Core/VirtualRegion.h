#pragma once
#include "Utils/NotCopyable.h"
#include <cstddef>
#include <type_traits>

class VirtualRegion : NotCopyable
{
public:

	VirtualRegion() = default;
	VirtualRegion(VirtualRegion&& other) noexcept;
	VirtualRegion& operator=(VirtualRegion&& other) noexcept;
	~VirtualRegion();

	bool reserve(std::size_t maxBytes);
	bool commitTo(std::size_t bytes);
	void decommitTo(std::size_t bytes);
	void release();

	std::byte* getData() const { return base_; }
	std::size_t getReservedBytes() const { return reserved_; }
	std::size_t getCommittedBytes() const { return committed_; }

	template <class T>
	T* getDataAs() const
	{
		static_assert(std::is_trivial_v<T>);
		return static_cast<T*>(static_cast<void*>(base_));
	}

private:

	std::byte* base_ = nullptr;
	std::size_t reserved_ = 0;
	std::size_t committed_ = 0;
};
