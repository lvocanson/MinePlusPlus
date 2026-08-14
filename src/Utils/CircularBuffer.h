#pragma once
#include <array>
#include <cassert>
#include <memory>
#include <utility>

template <class T, std::size_t Size>
class CircularBuffer
{
	std::size_t beg_{}, end_{};

	union
	{
		std::array<T, Size> buffer_;
	};

	std::size_t incr(std::size_t idx) const { return (idx + 1) % Size; }

public:

	~CircularBuffer()
	{
		empty();
	}

	bool isEmpty() const { return beg_ == end_; }
	bool isFull() const { return incr(end_) == beg_; }

	std::size_t size() const
	{
		if (end_ >= beg_)
			return end_ - beg_;
		else
			return Size - beg_ + end_;
	}

	template <typename... Args>
	void push(Args&&... args)
	{
		assert(!isFull());
		std::construct_at(&buffer_[end_], std::forward<Args>(args)...);
		end_ = incr(end_);
	}

	template <typename... Args>
	bool tryPush(Args&&... args)
	{
		auto idx = incr(end_);
		if (idx != beg_)
		{
			std::construct_at(&buffer_[end_], std::forward<Args>(args)...);
			end_ = idx;
			return true;
		}
		return false;
	}

	T pop()
	{
		assert(!isEmpty());
		auto idx = std::exchange(beg_, incr(beg_));
		T t = std::move(buffer_[idx]);
		std::destroy_at(&buffer_[idx]);
		return t;
	}

	void empty()
	{
		for (; beg_ != end_; incr(beg_))
			std::destroy_at(&buffer_[beg_]);
	}
};
