#include "CustomMenu.h"
#include "Core/App.h"
#include "Game/Minesweeper.h"
#include "UI/UITarget.h"
#include <format>

namespace
{

constexpr sf::Vector2i BUTTON_SIZE = {150, 28};
constexpr sf::Vector2i ELEMENTS_GAP = {200, 60};

}

CustomMenu::CustomMenu(Minesweeper& game)
	: game_(game)
	, widthText_{.origin = Text::Middle, .string = U"Width"}
	, heightText_{.origin = Text::Middle, .string = U"Height"}
	, minesText_{.origin = Text::Middle, .string = U"Mines"}
	, diagnosticText_{.origin = Text::Top}
	, widthField_{.rect = {{}, BUTTON_SIZE}}
	, heightField_{.rect = {{}, BUTTON_SIZE}}
	, minesField_{.rect = {{}, BUTTON_SIZE}}
	, backBtn_{.rect = {{}, BUTTON_SIZE}, .text = U"Back"}
	, startBtn_{.rect = {{}, BUTTON_SIZE}, .text = U"Start"}
{
	widthField_.setNumber(20);
	heightField_.setNumber(20);
	minesField_.setNumber(50);
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
		Vec2s newSize = {widthField_.toNumber(), heightField_.toNumber()};
		game_.resize(newSize);
		game_.setMineCount(minesField_.toNumber());
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

	Vec2s newSize = {widthField_.toNumber(), heightField_.toNumber()};
	std::size_t mineCount = minesField_.toNumber();

	std::size_t cellCount = newSize.x * newSize.y;
	std::size_t maxCells = std::vector<Cell>().max_size();

	diagnosticStr_.clear();
	if (!Board::isSizeValid(newSize))
	{
		if (!newSize.x || !newSize.y)
		{
			diagnosticStr_ += std::format("Invalid size: width or height is 0.\n");
		}
		else if (newSize.x > std::numeric_limits<std::size_t>::max() / newSize.y)
		{
			diagnosticStr_ += std::format("Invalid size: the number of cells is too big and cannot be computed.");
		}
		else if (cellCount > maxCells)
		{
			diagnosticStr_ += std::format("Invalid size: max number of cells exceeded: {}/{}\n", cellCount, maxCells);
		}
	}
	if (cellCount - 1 < mineCount)
	{
		diagnosticStr_ += std::format(
			"Max number of mines exceeded: {}/{}\n",
			mineCount,
			cellCount - 1);
	}
	diagnosticText_.string = {diagnosticStr_.getData(), diagnosticStr_.getSize()};

	return UIEvent::Consumed;
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
