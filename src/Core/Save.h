#pragma once
#include "Audio.h"
#include "Game/Minesweeper.h"
#include <chrono>
#include <cstdint>
#include <limits>
#include <optional>

struct Preferences
{
	static constexpr float MIN_SENSITIVITY = 0.1f;
	static constexpr float MAX_SENSITIVITY = 10.f;

	float volume = Audio::MAX_VOLUME / 2;
	float panSensitivity = 1.f;
	float zoomSensitivity = 1.f;
	std::uint32_t customWidth = 20;
	std::uint32_t customHeight = 20;
	std::uint32_t customMineCount = 50;
	Minesweeper::GameMode gameMode = Minesweeper::Default;
};

struct ScoreKey
{
	std::uint32_t width, height, mineCount;
	Minesweeper::GameMode gameMode;

	bool operator==(const ScoreKey&) const = default;
};

namespace Save
{

inline bool isStorable(std::size_t value) { return value <= std::numeric_limits<std::uint32_t>::max(); }

Preferences loadPreferences();
bool savePreferences(const Preferences& preferences);

std::optional<ScoreKey> makeScoreKey(const Minesweeper& game);
std::optional<std::chrono::milliseconds> loadBestTime(const ScoreKey& key);
bool submitTime(const ScoreKey& key, std::chrono::milliseconds time);
bool clearScores();

}
