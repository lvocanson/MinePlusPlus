#include "Minesweeper.h"

Minesweeper::Minesweeper()
	: state_{}
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
	state_ = Empty;
	resize({16, 16});
	setMineCount(40);
}

void Minesweeper::setHard()
{
	state_ = Empty;
	resize({30, 16});
	setMineCount(99);
}

void Minesweeper::resize(const Vec2s& size)
{
	if (!board_.isSizeValid(size))
		return;

	if (state_ == Playing)
		return;

	board_.resize(size);
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
	clock_.reset();
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
	}

	if (state_ != Playing)
		return;

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
