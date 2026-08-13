#pragma once
#include <SFML/Audio/Music.hpp>
#include <SFML/Audio/Sound.hpp>
#include <filesystem>
#include <optional>

/*
 * Fire and forget sound player.
 */
class Audio
{
public:

	static constexpr float MAX_VOLUME = 100.f; // SFML range: 0 (mute) to 100

	Audio();

	void play(const std::filesystem::path& file);
	void play(const sf::SoundBuffer& buffer);

	float getVolume() const { return volume_; }
	void setVolume(float volume);

private:

	sf::Music music_;
	std::optional<sf::Sound> sound_;
	float volume_;
};
