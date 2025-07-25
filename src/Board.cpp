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

bool Board::isSizeValid(const Vec2s& size) const
{
	if (size.x && size.y)
		// check overflow
		return size.x < std::numeric_limits<std::size_t>::max() / size.y
		// and respect max_size
		&& size.x * size.y < cells_.max_size();
	return false;
}

void Board::resize(const Vec2s& size)
{
	assert(isSizeValid(size));
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
	assert(mineCount <= getMaxNumberOfMines());
	mineCount_ = mineCount;
}

void Board::placeMines()
{
	assert(isSizeValid(size_));


	std::mt19937_64 gen(std::random_device{}());
	std::size_t noAdjCellCount = cells_.size();
	for (std::size_t i = cells_.size() - mineCount_; i < cells_.size(); ++i)
	{
		std::uniform_int_distribution<std::size_t> dist(0, i);
		std::size_t r = dist(gen);
		std::size_t index = cells_[r].mined ? i : r;

		assert(!cells_[index].mined);
		cells_[index].mined = true;

		for (auto coordinates : getNeigboursOf(toCoordinates(index)))
		{
			if (++cells_[toIndex(coordinates)].adjacentMines == 1)
				--noAdjCellCount;
		}
	}

	// 1 = square (more expensive), 0 = line (cheaper)
	double shape = double(std::min(size_.x, size_.y)) / std::max(size_.x, size_.y);
	double shapeCoef = .25, shapeBias = .5;
	
	// 1 = empty (more expensive), 0 = filled (no cost)
	double density = double(noAdjCellCount) / cells_.size();
	double densityCoef = .5, densityBias = .25;

	double largestFloodFillSize = noAdjCellCount
		* (shape * shapeCoef + shapeBias)
		* (density * densityCoef + densityBias);

	watchedIndices_.reserve(std::size_t(largestFloodFillSize) + 1);
	noAdj = noAdjCellCount;
	initWatched = watchedIndices_.capacity();
}

void Board::clear()
{
	assert(isSizeValid(size_));
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
		
		auto neighbours = getNeigboursOf(toCoordinates(index));
		for (auto coordinates : neighbours)
		{
			auto idx = toIndex(coordinates);
			auto& c = cells_[idx];
			if (c.flagged) ++flaggedNeighbourCount;
		};

		if (flaggedNeighbourCount != first.adjacentMines)
			return false;

		for (auto coordinates : neighbours)
		{
			auto idx = toIndex(coordinates);
			auto& c = cells_[idx];
			if (!c.opened && !c.flagged)
			{
				if (c.adjacentMines) openCell(c);
				else watch(idx);
			}
		}
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

		mineOpened |= openCell(cell);
		if (cell.adjacentMines)
			continue;

		for (auto coordinates : getNeigboursOf(toCoordinates(index)))
		{
			auto idx = toIndex(coordinates);
			auto& c = cells_[idx];
			if (!c.opened)
			{
				if (c.adjacentMines) openCell(c);
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

bool Board::openCell(Cell& cell)
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
	if (maxWatched < watchedIndices_.size())
	{
		maxWatched = watchedIndices_.size();
	}

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
