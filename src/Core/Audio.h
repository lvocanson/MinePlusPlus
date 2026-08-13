#pragma once
#include <SFML/Audio/Sound.hpp>
#include <array>
#include <cstddef>
#include <optional>

/*
 * Fire and forget sound player.
 */
class Audio
{
public:

	static constexpr std::size_t VOICE_COUNT = 5;
	static constexpr float MAX_VOLUME = 100.f; // SFML range: 0 (mute) to 100

	Audio();

	void play(const sf::SoundBuffer& buffer);

	float getVolume() const { return volume_; }
	void setVolume(float volume);

private:

	std::array<std::optional<sf::Sound>, VOICE_COUNT> voices_;
	std::size_t nextVoice_;
	float volume_;
};
