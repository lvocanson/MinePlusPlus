#pragma once
#include "Core/Canvas.h"
#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

class Board;
struct View;

namespace BoardRenderer
{

// How the board is revealed once the game is over.
enum class Reveal : std::uint8_t
{
	None, // game still running, nothing is revealed
	Lost, // every mine is shown, wrong flags are marked
	Won   // unflagged mines are shown, flags are kept
};

struct State
{
	Reveal reveal;
	std::optional<std::size_t> pressedCellIndex;
	const std::vector<std::size_t>& runningMineIndexes;
};

void render(Canvas canvas, const View& view, const Board& board, const State& state);

} // namespace BoardRenderer
