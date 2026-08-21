#include "CustomMenu.h"
#include "Core/App.h"
#include "Game/Minesweeper.h"
#include "UI/UITarget.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <format>
#include <iterator>

namespace
{

constexpr sf::Vector2i BUTTON_SIZE = {150, 28};
constexpr sf::Vector2i ELEMENTS_GAP = {200, 60};

// Difficulty derived from mine density, corrected for board size and shape.
//  - density : more mines per cell is harder (main factor)
//  - size    : a bigger board forces more decisions, slightly harder
//  - shape   : a square is the easiest shape, narrower is harder.
double difficultyScore(std::size_t width, std::size_t height, std::size_t mines)
{
	double cells = double(width) * double(height);
	if (cells <= 0.0)
		return 0.0;

	double density = double(mines) / cells;
	double sizeFactor = std::clamp(std::pow(cells / 256.0, 0.15), 0.8, 1.5);
	double minDim = double(std::min(width, height));
	double shapeFactor = 1.0 + 0.7 / minDim;

	return std::clamp(density * sizeFactor * shapeFactor, 0.0, 1.0);
}

std::string_view difficultyLabel(double score)
{
	if (score < 0.04) return "Free Win";
	if (score < 0.14) return "Easy";
	if (score < 0.18) return "Medium";
	if (score < 0.25) return "Hard";
	return "Impossible";
}

sf::Color difficultyColor(double score)
{
	struct Stop { double at; sf::Color color; };
	static constexpr Stop stops[] =
	{
		{0.020, {0x3B, 0x82, 0xF6}}, // Free Win   - blue
		{0.090, {0x22, 0xC5, 0x5E}}, // Easy       - green
		{0.160, {0xEA, 0xB3, 0x08}}, // Medium     - yellow
		{0.215, {0xEF, 0x44, 0x44}}, // Hard       - red
		{0.280, {0xA8, 0x55, 0xF7}}, // Impossible - purple
	};

	constexpr std::size_t n = std::size(stops);
	if (score <= stops[0].at) return stops[0].color;
	if (score >= stops[n - 1].at) return stops[n - 1].color;

	for (std::size_t i = 1; i < n; ++i)
	{
		if (score <= stops[i].at)
		{
			double t = (score - stops[i - 1].at) / (stops[i].at - stops[i - 1].at);
			const sf::Color& c0 = stops[i - 1].color;
			const sf::Color& c1 = stops[i].color;
			auto lerp = [t](std::uint8_t a, std::uint8_t b)
			{ return std::uint8_t(a + (int(b) - int(a)) * t); };
			return {lerp(c0.r, c1.r), lerp(c0.g, c1.g), lerp(c0.b, c1.b)};
		}
	}
	return stops[n - 1].color;
}

}

CustomMenu::CustomMenu(Minesweeper& game)
	: game_(game)
	, widthText_{.origin = Text::Middle, .string = "Width"}
	, heightText_{.origin = Text::Middle, .string = "Height"}
	, minesText_{.origin = Text::Middle, .string = "Mines"}
	, diagnosticText_{.origin = Text::Top}
	, widthField_{.rect = {{}, BUTTON_SIZE}, .value = 20}
	, heightField_{.rect = {{}, BUTTON_SIZE}, .value = 20}
	, minesField_{.rect = {{}, BUTTON_SIZE}, .value = 50}
	, backBtn_{.rect = {{}, BUTTON_SIZE}, .text = "Back"}
	, startBtn_{.rect = {{}, BUTTON_SIZE}, .text = "Start"}
{
	updateDiagnostic();
}

UIEvent::Result CustomMenu::operator()(const UIEvent::Resized& event)
{
	sf::Vector2i centerView = event.newSize / 2;
	widthText_.position = centerView - ELEMENTS_GAP;
	heightText_.position = centerView - sf::Vector2i(0, ELEMENTS_GAP.y);
	minesText_.position = centerView + sf::Vector2i(ELEMENTS_GAP.x, -ELEMENTS_GAP.y);
	diagnosticText_.position = centerView + sf::Vector2i(0, ELEMENTS_GAP.y * 2);
	widthField_.rect.position = centerView - widthField_.rect.size / 2 - sf::Vector2i(ELEMENTS_GAP.x, 0);
	heightField_.rect.position = centerView - heightField_.rect.size / 2;
	minesField_.rect.position = centerView - minesField_.rect.size / 2 + sf::Vector2i(ELEMENTS_GAP.x, 0);
	backBtn_.rect.position = centerView - backBtn_.rect.size / 2 + sf::Vector2i(-ELEMENTS_GAP.x, ELEMENTS_GAP.y);
	startBtn_.rect.position = centerView - startBtn_.rect.size / 2 + ELEMENTS_GAP;
	return UIEvent::Consumed;
}

UIEvent::Result CustomMenu::operator()(const UIEvent::Pressed& event)
{
	cursor_.field = nullptr;
	if (tracker_.registerPress(event.position, {backBtn_, startBtn_}))
		return UIEvent::Consumed;

	if (widthField_.rect.contains(event.position))
	{
		cursor_.field = &widthField_;
	}
	else if (heightField_.rect.contains(event.position))
	{
		cursor_.field = &heightField_;
	}
	else if (minesField_.rect.contains(event.position))
	{
		cursor_.field = &minesField_;
	}
	else
		return UIEvent::Ignored;
	return UIEvent::Consumed;
}

UIEvent::Result CustomMenu::operator()(const UIEvent::Released& event)
{
	if (tracker_.isClicked(backBtn_, event.position))
	{
		event.app.submitCommand<SwapUI>([&game = event.app.getGame()](AppUI& ui) { ui.emplace<PlayMenu>(game); });
	}
	else if (tracker_.isClicked(startBtn_, event.position))
	{
		Vec2s newSize = {widthField_.value, heightField_.value};
		game_.resize(newSize);
		game_.setMineCount(minesField_.value);
		event.app.submitCommand<SwapUI>([&app = event.app](AppUI& ui) { ui.emplace<GameUI>(app); });
		event.app.submitCommand<ChangeClearColor>(sf::Color{0x79, 0x31, 0x32, 0x00});
	}
	else
		return UIEvent::Ignored;
	return UIEvent::Consumed;
}

UIEvent::Result CustomMenu::operator()(const UIEvent::Typed& event)
{
	if (!cursor_.field)
		return UIEvent::Ignored;

	cursor_.field->add(event.utf32);
	updateDiagnostic();

	return UIEvent::Consumed;
}

void CustomMenu::updateDiagnostic()
{
	Vec2s newSize = {widthField_.value, heightField_.value};
	std::size_t mineCount = minesField_.value;

	std::size_t cellCount = newSize.x * newSize.y;
	std::size_t maxCells = std::vector<Cell>().max_size();

	diagnosticStr_.clear();
	auto out = std::back_inserter(diagnosticStr_);

	if (!Board::isSizeValid(newSize))
	{
		if (!newSize.x || !newSize.y)
		{
			std::format_to(out, "Invalid size: width or height is 0.\n");
		}
		else if (newSize.x > std::numeric_limits<std::size_t>::max() / newSize.y)
		{
			std::format_to(out, "Invalid size: the number of cells is too big and cannot be computed.\n");
		}
		else if (cellCount > maxCells)
		{
			std::format_to(out, "Invalid size: max number of cells exceeded: {}/{}\n", cellCount, maxCells);
		}
	}

	if (cellCount && cellCount - 1 < mineCount)
	{
		std::format_to(out, "Max number of mines exceeded: {}/{}\n", mineCount, cellCount - 1);
	}

	if (diagnosticStr_.empty())
	{
		double score = difficultyScore(newSize.x, newSize.y, mineCount);
		std::format_to(out, "Difficulty: {}", difficultyLabel(score));
		diagnosticText_.color = difficultyColor(score);
	}
	else
	{
		diagnosticText_.color = sf::Color::White;
	}

	diagnosticText_.string = diagnosticStr_;
}

void CustomMenu::render(UITarget& target) const
{
	target.draw(widthText_);
	target.draw(heightText_);
	target.draw(minesText_);
	target.draw(diagnosticText_);
	target.draw(widthField_);
	target.draw(heightField_);
	target.draw(minesField_);
	target.draw(backBtn_);
	target.draw(startBtn_);
	target.draw(cursor_);
}
