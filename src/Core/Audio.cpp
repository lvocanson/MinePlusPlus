#include "Audio.h"
#include <SFML/Audio/SoundBuffer.hpp>

Audio::Audio()
	: voices_{}
	, nextVoice_{}
	, volume_(MAX_VOLUME / 2)
{
}

void Audio::play(const sf::SoundBuffer& buffer)
{
	std::optional<sf::Sound>& voice = voices_[nextVoice_];
	nextVoice_ = (nextVoice_ + 1) % VOICE_COUNT;

	if (voice)
	{
		voice->stop();
		voice->setBuffer(buffer);
	}
	else
	{
		voice.emplace(buffer);
		voice->setVolume(volume_);
	}

	voice->play();
}

void Audio::setVolume(float volume)
{
	volume_ = volume;
	for (std::optional<sf::Sound>& voice : voices_)
	{
		if (voice)
			voice->setVolume(volume_);
	}
}
