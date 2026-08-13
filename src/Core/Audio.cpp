#include "Audio.h"
#include <SFML/Audio/SoundBuffer.hpp>

Audio::Audio()
	: volume_(MAX_VOLUME / 2)
{
	music_.setVolume(volume_);
}

void Audio::play(const std::filesystem::path& file)
{
	if (music_.openFromFile(file))
	{
		music_.play();
	}
}

void Audio::play(const sf::SoundBuffer& buffer)
{
	if (sound_)
	{
		sound_->stop();
		sound_->setBuffer(buffer);
	}
	else
	{
		sound_.emplace(buffer).setVolume(volume_);
	}

	sound_->play();
}

void Audio::setVolume(float volume)
{
	volume_ = volume;
	music_.setVolume(volume);

	if (sound_)
		sound_->setVolume(volume);
}
