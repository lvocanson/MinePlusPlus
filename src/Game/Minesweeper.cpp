#include "Minesweeper.h"
#include "Utils/MyRandom.h"
#include "Core/View.h"
#include <cassert>
#include <numbers>

Minesweeper::Minesweeper()
	: rendering_(false)
	, cheated_(false)
	, state_{}
	, gameMode_{}
	, rotationSpeed_{}
{
	clock_.reset();
}

void Minesweeper::setEasy()
{
	resize({9, 9});
	setMineCount(10);
}

void Minesweeper::setMedium()
{
	resize({16, 16});
	setMineCount(40);
}

void Minesweeper::setHard()
{
	resize({30, 16});
	setMineCount(99);
}

void Minesweeper::resize(const Vec2s& size)
{
	if (!board_.isSizeValid(size))
		return;

	board_.resize(size);
	runningBombIndexes_.clear();
	state_ = Empty;
}

void Minesweeper::setMineCount(std::size_t mineCount)
{
	if (mineCount > board_.getMaxNumberOfMines())
		return;

	board_.setMineCount(mineCount);
}

void Minesweeper::restart()
{
	// If resize was not called once, then the size is invalid (0, 0)
	if (!board_.isSizeValid(board_.getSize()))
	{
		state_ = Empty;
		return;
	}

	board_.clear();
	board_.placeMines();
	// Indexes of the previous game no longer point to mines
	runningBombIndexes_.clear();
	clock_.reset();
	cheated_ = false;
	state_ = Ready;
}

void Minesweeper::open(const Vec2s& coordinates)
{
	if (state_ == Empty)
		restart();

	if (!board_.areCoordinatesValid(coordinates))
		return;

	std::size_t index = board_.toIndex(coordinates);

	if (state_ == Ready)
	{
		// First click
		board_.makeSafe(index);
		clock_.restart();
		state_ = Playing;
		randomizeRunningBombIndexes();
	}

	if (state_ != Playing)
		return;

	std::size_t openCount = board_.getOpenCount();
	if (board_.open(index))
	{
		state_ = Lost;
		clock_.stop();
	}
	else if (board_.isWon())
	{
		state_ = Won;
		clock_.stop();
	}
	else if (openCount < board_.getOpenCount())
	{
		// Move the mine at each revealing click
		for (auto& rbIdx : runningBombIndexes_)
			rbIdx = board_.moveMine(rbIdx);
	}
}

void Minesweeper::flag(const Vec2s& coordinates)
{
	if (state_ == Empty)
		restart();

	if (state_ != Ready && state_ != Playing)
		return;

	if (!board_.areCoordinatesValid(coordinates))
		return;

	std::size_t index = board_.toIndex(coordinates);
	board_.flag(index);
}

#ifdef MPP_CHEATS
void Minesweeper::cheatRevealNumbers()
{
	if (!rendering_ || state_ != Playing)
		return;

	cheated_ = true;
	board_.revealNumbers();
	if (board_.isWon())
	{
		state_ = Won;
		clock_.stop();
	}
}

void Minesweeper::cheatFlagMines()
{
	if (!rendering_ || state_ != Playing)
		return;

	cheated_ = true;
	board_.flagMinesOnly();
}
#endif

void Minesweeper::dispatchWorldEvent(const WorldEvent& event)
{
	if (rendering_)
	{
		event.visit(controls_);
	}
}

void Minesweeper::update(View& view, float dt)
{
	if (!rendering_ || rotationSpeed_ == 0.f)
		return;

	Vec2d center = Vec2d(board_.getSize()) / 2.0;
	view.rotateAround(center, rotationSpeed_ * dt * std::numbers::pi_v<float> / 180.f);
}

void Minesweeper::render(Canvas canvas, const View& view) const
{
	if (!rendering_)
		return;

	std::optional<std::size_t> pressedCellIndex;
	if (pressedCell_)
		pressedCellIndex = board_.toIndex(*pressedCell_);

	auto reveal = state_ == Lost
	              ? BoardRenderer::Reveal::Lost
	              : state_ == Won
	                ? BoardRenderer::Reveal::Won
	                : BoardRenderer::Reveal::None;

	BoardRenderer::State state
	{
		.reveal             = reveal,
		.pressedCellIndex   = pressedCellIndex,
		.runningMineIndexes = runningBombIndexes_
	};
	BoardRenderer::render(canvas, view, board_, state);
}

void Minesweeper::resetParameters()
{
	setPressedCell(std::nullopt);
	setRendering(false);
	setGameMode(Default);
}

void Minesweeper::setGameMode(GameMode mode)
{
	gameMode_ = mode;
	setRotationSpeed(mode == Spinning ? 1.f : 0.f);
}

void Minesweeper::setPressedCell(std::optional<Vec2s> coordinates)
{
	pressedCell_ = coordinates;
}

void Minesweeper::randomizeRunningBombIndexes()
{
	std::size_t minesLeftToIterate = board_.getMineCount();
	std::size_t count = gameMode_ == RunningBomb ? 1 + minesLeftToIterate / 40 : 0;
	runningBombIndexes_.assign(std::min(count, minesLeftToIterate), 0);

	std::size_t minesLeftToChoose = runningBombIndexes_.size();

	auto& cells = board_.getCells();
	for (std::size_t i = 0; i < cells.size() && minesLeftToChoose; ++i)
	{
		if (!cells[i].mined)
			continue;

		std::uniform_int_distribution<std::size_t> dist(0, minesLeftToIterate - 1);
		if (dist(gen) < minesLeftToChoose)
			runningBombIndexes_[--minesLeftToChoose] = i;
		--minesLeftToIterate;
	}
}
