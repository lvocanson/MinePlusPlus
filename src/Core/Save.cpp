#include "Save.h"
#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <limits>
#include <memory>
#include <span>
#include <type_traits>
#include <vector>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#else
#include <cstdlib>
#endif

namespace
{

struct ScoreRecord
{
	ScoreKey key;
	std::uint32_t bestMs;
};

// TODO: raw structs are written as is, the files are only portable between little-endian platforms
static_assert(std::is_trivially_copyable_v<Preferences> && sizeof(Preferences) == 28);
static_assert(std::is_trivially_copyable_v<ScoreRecord> && sizeof(ScoreRecord) == 20);

constexpr const char* PREFERENCES_FILE = "preferences.bin";
constexpr const char* SCORES_FILE = "scores.bin";
constexpr std::uintmax_t MAX_FILE_SIZE = 1 << 20;

bool isValid(const Preferences& preferences)
{
	auto isValidSensitivity = [](float sensitivity)
	{ return sensitivity >= Preferences::MIN_SENSITIVITY && sensitivity <= Preferences::MAX_SENSITIVITY; };

	return preferences.volume >= 0.f && preferences.volume <= Audio::MAX_VOLUME
	       && isValidSensitivity(preferences.panSensitivity)
	       && isValidSensitivity(preferences.zoomSensitivity)
	       && preferences.gameMode < Minesweeper::Count;
}

bool isValid(const ScoreRecord& record)
{
	const ScoreKey& key = record.key;
	return key.width && key.height
	       && key.mineCount < std::uint64_t(key.width) * key.height
	       && key.gameMode < Minesweeper::Count;
}

std::filesystem::path savePath(const char* fileName)
{
#ifdef _WIN32
	wchar_t appData[MAX_PATH];
	DWORD length = GetEnvironmentVariableW(L"APPDATA", appData, MAX_PATH);
	if (length == 0 || length >= MAX_PATH)
		return {};
	return std::filesystem::path(appData) / L"Mine++" / fileName;
#else
	if (const char* dataHome = std::getenv("XDG_DATA_HOME"); dataHome && *dataHome)
		return std::filesystem::path(dataHome) / "Mine++" / fileName;
	if (const char* home = std::getenv("HOME"); home && *home)
		return std::filesystem::path(home) / ".local/share/Mine++" / fileName;
	return {};
#endif
}

struct FileCloser
{
	void operator()(std::FILE* file) const { std::fclose(file); }
};

using File = std::unique_ptr<std::FILE, FileCloser>;

File openFile(const std::filesystem::path& path, bool write)
{
#ifdef _WIN32
	std::FILE* file = nullptr;
	_wfopen_s(&file, path.c_str(), write ? L"wb" : L"rb");
	return File(file);
#else
	return File(std::fopen(path.c_str(), write ? "wb" : "rb"));
#endif
}

template <class T>
std::vector<T> readFile(const std::filesystem::path& path)
{
	if (path.empty())
		return {};

	std::error_code error;
	auto size = std::filesystem::file_size(path, error);
	if (error || size < sizeof(T) || size > MAX_FILE_SIZE)
		return {};

	std::vector<T> records(std::size_t(size / sizeof(T)));
	File file = openFile(path, false);
	if (!file || std::fread(records.data(), sizeof(T), records.size(), file.get()) != records.size())
		return {};
	std::erase_if(records, [](const T& record) { return !isValid(record); });
	return records;
}

template <class T>
bool writeFile(const std::filesystem::path& path, std::span<const T> records)
{
	if (path.empty())
		return false;

	std::error_code error;
	std::filesystem::create_directories(path.parent_path(), error);
	if (error)
		return false;

	auto temporary = path;
	temporary += ".tmp";
	File file = openFile(temporary, true);
	if (!file)
		return false;

	bool written = records.empty()
	               || std::fwrite(records.data(), sizeof(T), records.size(), file.get()) == records.size();
	bool closed = std::fclose(file.release()) == 0;
	if (!written || !closed)
	{
		std::filesystem::remove(temporary, error);
		return false;
	}

	std::filesystem::rename(temporary, path, error);
	return !error;
}

} // namespace

Preferences Save::loadPreferences()
{
	auto records = readFile<Preferences>(savePath(PREFERENCES_FILE));
	return records.size() == 1 ? records.front() : Preferences{};
}

bool Save::savePreferences(const Preferences& preferences)
{
	return writeFile(savePath(PREFERENCES_FILE), std::span(&preferences, 1));
}

std::optional<ScoreKey> Save::makeScoreKey(const Minesweeper& game)
{
	auto& board = game.getBoard();
	Vec2s size = board.getSize();
	if (!isStorable(size.x) || !isStorable(size.y) || !isStorable(board.getMineCount()))
		return std::nullopt;

	return ScoreKey
	{
		.width     = std::uint32_t(size.x),
		.height    = std::uint32_t(size.y),
		.mineCount = std::uint32_t(board.getMineCount()),
		.gameMode  = game.getGameMode()
	};
}

std::optional<std::chrono::milliseconds> Save::loadBestTime(const ScoreKey& key)
{
	auto records = readFile<ScoreRecord>(savePath(SCORES_FILE));
	auto it = std::ranges::find(records, key, &ScoreRecord::key);
	if (it == records.end())
		return std::nullopt;
	return std::chrono::milliseconds(it->bestMs);
}

bool Save::submitTime(const ScoreKey& key, std::chrono::milliseconds time)
{
	if (time.count() < 0 || time.count() > std::numeric_limits<std::uint32_t>::max())
		return false;

	auto ms = std::uint32_t(time.count());
	auto records = readFile<ScoreRecord>(savePath(SCORES_FILE));
	auto it = std::ranges::find(records, key, &ScoreRecord::key);
	if (it == records.end())
		records.push_back({key, ms});
	else if (ms < it->bestMs)
		it->bestMs = ms;
	else
		return false;

	return writeFile(savePath(SCORES_FILE), std::span<const ScoreRecord>(records));
}

bool Save::clearScores()
{
	auto path = savePath(SCORES_FILE);
	if (path.empty())
		return false;

	std::error_code error;
	std::filesystem::remove(path, error);
	return !error;
}
