#pragma once
#include "Utils/NotCopyable.h"
#include "Utils/NotMovable.h"
#include <chrono>
#include <cstdint>
#include <memory>
#include <span>

/*
 * Fire and forget sound player.
 */
class Audio : NotCopyable, NotMovable
{
public:

	enum class Track : std::uint8_t
	{
		Effect,
		Music,
		Count
	};

	struct Sound
	{
		std::span<const unsigned char> wav;
		Track track;
	};

	static constexpr float MAX_VOLUME = 100.f; // 0 (mute) to 100

	Audio();
	~Audio();

	void play(const Sound& sound);
	void waitUntilIdle(Track track, std::chrono::milliseconds timeout) const;

	float getVolume() const { return volume_; }
	void setVolume(float volume);

private:

	struct State;
	std::unique_ptr<State> state_;
	float volume_;
};
