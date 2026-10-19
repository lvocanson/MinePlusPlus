#pragma once
#include <chrono>
#include <optional>
#include <utility>

class Stopwatch
{
public:

	using Clock = std::chrono::steady_clock;

	void reset()
	{
		elapsed_ = {};
		start_.reset();
	}

	void restart()
	{
		elapsed_ = {};
		start_ = Clock::now();
	}

	void stop()
	{
		if (start_)
			elapsed_ += Clock::now() - *std::exchange(start_, std::nullopt);
	}

	Clock::duration getElapsedTime() const
	{
		return start_ ? elapsed_ + (Clock::now() - *start_) : elapsed_;
	}

private:

	Clock::duration elapsed_{};
	std::optional<Clock::time_point> start_;
};
