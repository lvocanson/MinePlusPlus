#include "Audio.h"
#include <miniaudio.h>
#include <algorithm>
#include <array>
#include <atomic>
#include <mutex>
#include <thread>

#ifdef MPP_PROFILING
#include "Profiling/MiniaudioAllocators.h"
#endif

struct Audio::State
{
	static constexpr ma_uint32 OUTPUT_CHANNELS = 2;
	static constexpr ma_uint32 MIX_FRAMES = 256;

	struct Voice
	{
		ma_decoder decoder;
		const unsigned char* source = nullptr;
		std::atomic<bool> playing = false;
	};

	ma_device device;
	bool deviceReady = false;
	std::mutex mutex;
	std::array<Voice, std::size_t(Track::Count)> voices{};

	static void mix(Voice& voice, float* out, ma_uint32 frameCount)
	{
		std::array<float, MIX_FRAMES * OUTPUT_CHANNELS> scratch;
		while (frameCount > 0 && voice.playing)
		{
			ma_uint64 toRead = std::min(frameCount, MIX_FRAMES);
			ma_uint64 read = 0;
			ma_decoder_read_pcm_frames(&voice.decoder, scratch.data(), toRead, &read);

			for (std::size_t i = 0; i < read * OUTPUT_CHANNELS; ++i)
				out[i] += scratch[i];

			voice.playing = read == toRead;
			out += read * OUTPUT_CHANNELS;
			frameCount -= ma_uint32(read);
		}
	}

	static void onData(ma_device* device, void* output, const void*, ma_uint32 frameCount)
	{
		auto& state = *static_cast<State*>(device->pUserData);

		// TODO: a play() in progress silences this whole period, a lock-free handoff would not
		std::unique_lock lock(state.mutex, std::try_to_lock);
		if (!lock)
			return;

		for (Voice& voice : state.voices)
			mix(voice, static_cast<float*>(output), frameCount);
	}

	~State()
	{
		if (deviceReady)
			ma_device_uninit(&device);

		for (Voice& voice : voices)
			if (voice.source)
				ma_decoder_uninit(&voice.decoder);
	}
};

Audio::Audio()
	: state_(std::make_unique<State>())
	, volume_(MAX_VOLUME / 2)
{
	ma_device_config config = ma_device_config_init(ma_device_type_playback);
	config.playback.format = ma_format_f32;
	config.playback.channels = State::OUTPUT_CHANNELS;
	config.dataCallback = State::onData;
	config.pUserData = state_.get();

#ifdef MPP_PROFILING
	ma_context_config contextConfig = ma_context_config_init();
	contextConfig.allocationCallbacks = Profiling::makeMiniaudioCallbacks("miniaudio device");
	if (ma_device_init_ex(nullptr, 0, &contextConfig, &config, &state_->device) != MA_SUCCESS)
		return;
#else
	if (ma_device_init(nullptr, &config, &state_->device) != MA_SUCCESS)
		return;
#endif

	state_->deviceReady = true;
	setVolume(volume_);

	if (ma_device_start(&state_->device) != MA_SUCCESS)
	{
		ma_device_uninit(&state_->device);
		state_->deviceReady = false;
	}
}

Audio::~Audio() = default;

void Audio::play(const Sound& sound)
{
	if (!state_->deviceReady)
		return;

	std::lock_guard lock(state_->mutex);
	State::Voice& voice = state_->voices[std::size_t(sound.track)];

	if (voice.source == sound.wav.data())
	{
		voice.playing = ma_decoder_seek_to_pcm_frame(&voice.decoder, 0) == MA_SUCCESS;
		return;
	}

	if (voice.source)
		ma_decoder_uninit(&voice.decoder);

	ma_decoder_config config = ma_decoder_config_init(ma_format_f32, State::OUTPUT_CHANNELS, state_->device.sampleRate);
#ifdef MPP_PROFILING
	config.allocationCallbacks = Profiling::makeMiniaudioCallbacks(
		sound.track == Track::Effect ? "miniaudio effect decoder" : "miniaudio music decoder");
#endif
	bool loaded = ma_decoder_init_memory(sound.wav.data(), sound.wav.size(), &config, &voice.decoder) == MA_SUCCESS;
	voice.source = loaded ? sound.wav.data() : nullptr;
	voice.playing = loaded;
}

void Audio::waitUntilIdle(Track track, std::chrono::milliseconds timeout) const
{
	if (!state_->deviceReady)
		return;

	const State::Voice& voice = state_->voices[std::size_t(track)];
	auto deadline = std::chrono::steady_clock::now() + timeout;
	while (voice.playing && std::chrono::steady_clock::now() < deadline)
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

void Audio::setVolume(float volume)
{
	volume_ = volume;
	if (state_->deviceReady)
		ma_device_set_master_volume(&state_->device, volume / MAX_VOLUME);
}
