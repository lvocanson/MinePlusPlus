#include "Board.h"
#include <algorithm>
#include <cassert>
#include <random>

Board::Board()
	: size_{}
	, mineCount_{}, flagCount_{}, openCount_{}
	, cells_{}, watchedIndices_{}
{
}

void Board::resize(const Vec2s& size)
{
	size_ = size;
	clear();
}

bool Board::areCoordinatesValid(const Vec2s& coordinates) const
{
	// coordinates are unsigned, no need to check >= 0
	return coordinates.x < size_.x && coordinates.y < size_.y;
}

bool Board::isIndexValid(std::size_t index) const
{
	return index < cells_.size();
}

std::size_t Board::toIndex(const Vec2s& coordinates) const
{
	assert(areCoordinatesValid(coordinates));
	return coordinates.x + coordinates.y * size_.x;
}

Vec2s Board::toCoordinates(std::size_t index) const
{
	assert(isIndexValid(index));
	return
	{
		.x = index % size_.x,
		.y = index / size_.x
	};
}

void Board::setMineCount(std::size_t mineCount)
{
	assert(mineCount > 0);
	assert(mineCount <= getMaxNumberOfMines());
	mineCount_ = mineCount;
}

void Board::placeMines()
{
	clear();

	std::mt19937_64 gen(std::random_device{}());
	std::size_t cellsWithoutAdjacentMines = cells_.size();

	for (std::size_t i = cells_.size() - mineCount_; i < cells_.size(); ++i)
	{
		std::uniform_int_distribution<std::size_t> dis(0, i);
		std::size_t r = dis(gen);
		std::size_t index = cells_[r].mined ? i : r;

		assert(!cells_[index].mined);
		cells_[index].mined = true;

		for (auto coordinates : getNeigboursOf(toCoordinates(index)))
		{
			if (++cells_[toIndex(coordinates)].adjacentMines == 1)
				--cellsWithoutAdjacentMines;
		}
	}
}

void Board::clear()
{
	flagCount_ = openCount_ = 0;
	cells_.assign(size_.x * size_.y, {});
	watchedIndices_.clear();
}

bool Board::open(std::size_t index)
{
	assert(isIndexValid(index));

	auto& first = cells_[index];
	if (first.flagged)
		return false;

	if (first.opened)
	{
		// If re-opening a cell then open all neighbours, but only if the
		// number of flagged neighbours match the number of adjacent mines.

		std::size_t flaggedNeighbourCount = 0;
		for (auto coordinates : getNeigboursOf(toCoordinates(index)))
		{
			auto idx = toIndex(coordinates);
			auto& c = cells_[idx];
			if (c.flagged) ++flaggedNeighbourCount;
			else if (!c.opened) watch(idx);
		};

		if (flaggedNeighbourCount != first.adjacentMines)
			return false;
	}
	else
	{
		watch(index);
	}

	bool mineOpened = false;
	while (!watchedIndices_.empty())
	{
		index = popLastWatched();
		auto& cell = cells_[index];

		mineOpened |= open_single(cell);
		if (cell.adjacentMines)
			continue;

		for (auto coordinates : getNeigboursOf(toCoordinates(index)))
		{
			auto idx = toIndex(coordinates);
			auto& c = cells_[idx];
			if (!c.opened)
			{
				if (c.adjacentMines)open_single(c);
				else watch(idx);
			}
		};
	}
	return mineOpened;
}

void Board::flag(std::size_t index)
{
	assert(isIndexValid(index));
	auto& cell = cells_[index];
	if (!cell.opened)
		flagCount_ += std::size_t(cell.flagged ^= true) * 2 - 1;
}

bool Board::open_single(Cell& cell)
{
	cell.opened = true;
	flagCount_ -= cell.flagged;
	cell.flagged = false;
	++openCount_;
	return cell.mined;
}

void Board::watch(std::size_t index)
{
	auto& cell = cells_[index];
	if (!cell.watched)
	{
		cell.watched = true;
		watchedIndices_.emplace_back(index);
	}
}

std::size_t Board::popLastWatched()
{
	assert(!watchedIndices_.empty());
	std::size_t index = watchedIndices_.back();
	watchedIndices_.pop_back();
	cells_[index].watched = false;
	return index;
}

NeighbourRange::NeighbourRange(const Board& board, const Vec2s& coordinates)
	: board(board)
	, coordinates(coordinates)
{
}

NeighbourRange::Iterator::Iterator(const NeighbourRange* range)
	: range(range)
	, dc(-2), dr(-1)
{
	operator++();
}

NeighbourRange::Iterator& NeighbourRange::Iterator::operator++()
{
	while (range)
	{
		if (++dc > 1)
		{
			dc = -1;
			++dr;
		}
		if (dr > 1)
		{
			range = nullptr;
			break;
		}
		if (range->board.areCoordinatesValid(**this))
			break;
	}
	return *this;
}

Vec2s NeighbourRange::Iterator::operator*() const
{
	Vec2s retval = range->coordinates;
	retval.x += dc;
	retval.y += dr;
	return retval;
}
