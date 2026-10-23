#pragma once
#include "Core/Canvas.h"
#include "UI/Font.h"
#include <algorithm>
#include <array>
#include <cstddef>
#include <format>
#include <string_view>
#include <utility>

namespace Profiling::Draw
{

constexpr Color BACKGROUND_COLOR = {0x18, 0x18, 0x1B};
constexpr Color GRID_COLOR = {0x3F, 0x3F, 0x46};
constexpr Color TEXT_COLOR = {0xD4, 0xD4, 0xD8};
constexpr Color MUTED_COLOR = {0x71, 0x71, 0x7A};
constexpr Color BUTTON_COLOR = {0x27, 0x27, 0x2A};
constexpr Color ACTIVE_BUTTON_COLOR = {0x3F, 0x3F, 0x46};
constexpr int MARGIN = 12;
constexpr int BUTTON_PADDING = 6;
constexpr int BUTTON_GAP = 8;
constexpr int TEXT_BASELINE_OFFSET = 7;

class Label
{
public:

	template <class... Args>
	std::string_view operator()(std::format_string<Args...> fmt, Args&&... args)
	{
		auto result = std::format_to_n(buffer_.data(), buffer_.size(), fmt, std::forward<Args>(args)...);
		return {buffer_.data(), std::min(std::size_t(result.size), buffer_.size())};
	}

private:

	std::array<char, 128> buffer_;
};

const Font& font();
int lineHeight();
float textWidth(std::string_view text);
std::string_view fitText(std::string_view text, float maxWidth);
void drawText(Canvas canvas, Vec2f baseline, std::string_view text, Color color);
void drawLine(Canvas canvas, Vec2i from, Vec2i to, Color color);
Recti drawButton(Canvas canvas, Vec2i position, std::string_view text, bool active);

} // namespace Profiling::Draw
