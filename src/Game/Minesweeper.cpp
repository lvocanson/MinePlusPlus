#include "Minesweeper.h"
#include "Utils/MyRandom.h"
#include <cassert>

Minesweeper::Minesweeper()
	: rendering_(false)
	, state_{}
	, rotationSpeed_{}
	, runningBombCount_{}
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
	renderer_.resize(board_);
	runningBombIndexes_.clear();
	state_ = Empty;
	renderer_.makeDirty();
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
	state_ = Ready;
	renderer_.makeDirty();
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
		for (auto& index : runningBombIndexes_)
			index = board_.moveMine(index);
	}
	renderer_.makeDirty();
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
	renderer_.makeDirty();
}

void Minesweeper::dispatchWorldEvent(const WorldEvent& event)
{
	if (rendering_)
	{
		event.visit(controls_);
	}
}

void Minesweeper::update(float dt)
{
	frameRotation_ = sf::degrees(rotationSpeed_ * dt);

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
	renderer_.update(board_, state);
}

void Minesweeper::render(sf::RenderTarget& target) const
{
	if (!rendering_)
		return;

	sf::View view = target.getView();
	sf::Vector2f center = {board_.getSize().x / 2.f, board_.getSize().y / 2.f};
	sf::Vector2f offset = center - view.getCenter();
	view.move(offset - offset.rotatedBy(frameRotation_));
	view.rotate(frameRotation_);
	target.setView(view);

	renderer_.render(target);
}

void Minesweeper::resetParameters()
{
	setPressedCell(std::nullopt);
	setRendering(false);
	setRotationSpeed(0.f);
	setRunningBombCount(0);
}

void Minesweeper::setPressedCell(std::optional<Vec2s> coordinates)
{
	pressedCell_ = coordinates;
	renderer_.makeDirty();
}

void Minesweeper::setRunningBombCount(std::size_t count)
{
	// Clamped at randomization time, the mine count may still change
	runningBombCount_ = count;
}

void Minesweeper::randomizeRunningBombIndexes()
{
	std::size_t minesLeftToIterate = board_.getMineCount();
	runningBombIndexes_.assign(std::min(runningBombCount_, minesLeftToIterate), 0);

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
