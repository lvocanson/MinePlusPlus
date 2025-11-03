#include "Board.h"
#include "Utils/MyRandom.h"
#include <algorithm>
#include <cassert>

constexpr Vec2s Vec2s::operator+(const Vec2s& rhs) const
{
	return {x + rhs.x, y + rhs.y};
}

namespace
{
constexpr NeighbourRange::nt dcoordinates
{{
	{-1, -1},
	{ 0, -1},
	{ 1, -1},
	{-1,  0},
	{ 0,  0},
	{ 1,  0},
	{-1,  1},
	{ 0,  1},
	{ 1,  1}
}};
}

NeighbourRange::NeighbourRange(const Board& board, const Vec2s& coordinates)
{
	std::size_t n = 0;
	for (auto& dc : dcoordinates)
	{
		auto coo = coordinates + dc;
		if (board.areCoordinatesValid(coo))
			neighbours[n++] = coo;
	}

	for (; n < neighbours.size(); ++n)
	{
		neighbours[n] = INVALID_VEC2S;
	}
}

NeighbourRange::nt::const_iterator NeighbourRange::begin() const
{
	return neighbours.begin();
}

NeighbourRange::nt::const_iterator NeighbourRange::end() const
{
	for (auto it = neighbours.begin(); it != neighbours.end(); ++it)
	{
		if (*it == INVALID_VEC2S)
			return it;
	}
	return neighbours.end();
}

Board Board::EasyBoard()
{
	Board b;
	b.resize({9, 9});
	b.setMineCount(10);
	return b;
}

Board Board::MediumBoard()
{
	Board b;
	b.resize({16, 16});
	b.setMineCount(40);
	return b;
}

Board Board::HardBoard()
{
	Board b;
	b.resize({30, 16});
	b.setMineCount(99);
	return b;
}

Board::Board()
	: size_{}
	, mineCount_{}, flagCount_{}, openCount_{}
	, cells_{}
	, begFrontline_{}, endFrontline_{}
	, frontline_{}
{
}

bool Board::isSizeValid(const Vec2s& size)
{
	if (size.x && size.y)
		// check overflow
		return size.x < std::numeric_limits<std::size_t>::max() / size.y
		// and respect max_size
		&& size.x * size.y < std::vector<Cell>().max_size();
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

	for (std::size_t i = cells_.size() - mineCount_; i < cells_.size(); ++i)
	{
		std::uniform_int_distribution<std::size_t> dist(0, i);
		std::size_t r = dist(gen);
		std::size_t index = cells_[r].mined ? i : r;
		mineCell(index);
	}
}

void Board::makeSafe(std::size_t index)
{
	assert(isIndexValid(index));

	auto& cell = cells_[index];
	if (!cell.mined)
		return;

	// mine another cell
	std::size_t spotsLeft = cells_.size() - mineCount_;
	std::uniform_int_distribution<std::size_t> dist(1, spotsLeft);
	std::size_t n = dist(gen);
	for (std::size_t i = 0; i < cells_.size(); ++i)
	{
		if (!cells_[i].mined && --n == 0)
			mineCell(i);
	}

	// remove this mine
	cell.mined = false;
	for (auto& coordinates : getNeigboursOf(toCoordinates(index)))
	{
		--cells_[toIndex(coordinates)].adjacentMines;
	}
}

void Board::clear()
{
	assert(isSizeValid(size_));
	flagCount_ = openCount_ = 0;
	cells_.assign(size_.x * size_.y, {});
	setupFrontline();
}

bool Board::open(std::size_t index)
{
	assert(isIndexValid(index));

	auto& first = cells_[index];
	if (first.flagged)
		return false;
	
	bool mineOpened = false;

	if (first.opened)
	{
		// If re-opening a cell then open all neighbours, but only if the
		// number of flagged neighbours match the number of adjacent mines.
		std::size_t flaggedNeighbourCount = 0;

		auto neighbours = getNeigboursOf(toCoordinates(index));
		for (auto& coordinates : neighbours)
		{
			auto idx = toIndex(coordinates);
			auto& c = cells_[idx];
			if (c.flagged) ++flaggedNeighbourCount;
		};

		if (flaggedNeighbourCount != first.adjacentMines)
			return false;

		for (auto& coordinates : neighbours)
		{
			auto idx = toIndex(coordinates);
			auto& c = cells_[idx];
			if (!c.opened && !c.flagged)
			{
				if (c.adjacentMines) mineOpened |= openCell(c);
				else pushFrontline(idx);
			}
		}
	}

	else
	{
		pushFrontline(index);
	}

	// BFS algorithm. We maintain a 'frontline', expanding towards closed and
	// no adjacentMines cells. A 'frontline' cell is a cell that will soon be
	// opened, to avoid duplicates in the frontline_ queue/circular array.
	while (!isFrontlineEmpty())
	{
		index = popFrontline();
		auto& cell = cells_[index];

		mineOpened |= openCell(cell);
		if (cell.adjacentMines)
			continue;

		for (auto& coordinates : getNeigboursOf(toCoordinates(index)))
		{
			auto idx = toIndex(coordinates);
			auto& c = cells_[idx];
			if (!c.opened)
			{
				if (c.adjacentMines)
				{
					bool shouldNotHaveOpenedAMine = openCell(c);
					assert(!shouldNotHaveOpenedAMine);
				}
				else if (!c.frontline)
				{
					pushFrontline(idx);
				}
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
	{
		flagCount_ += std::size_t(cell.flagged ^= true) * 2 - 1;
	}
}

void Board::mineCell(std::size_t index)
{
	assert(isIndexValid(index));
	assert(!cells_[index].mined);
	cells_[index].mined = true;
	for (auto& coordinates : getNeigboursOf(toCoordinates(index)))
	{
		++cells_[toIndex(coordinates)].adjacentMines;
	}
}

bool Board::openCell(Cell& cell)
{
	assert(!cell.opened);
	cell.opened = true;
	flagCount_ -= cell.flagged;
	cell.flagged = false;
	++openCount_;
	return cell.mined;
}

void Board::setupFrontline()
{
	std::size_t fSize = biggestFrontline() / 4;
	frontline_.resize(fSize);
	begFrontline_ = endFrontline_ = 0;
}

std::size_t Board::biggestFrontline() const
{
	// The frontline expands as a square (8 neighbours) so in the worst case,
	// meaning no mines to stop it, the biggest square perimeter is four times
	// the smallest side, minus duplicates cells.
	return std::min(size_.x, size_.y) * 4 - 2;
}

bool Board::isFrontlineEmpty() const
{
	return begFrontline_ == endFrontline_;
}

void Board::pushFrontline(std::size_t index)
{
	assert(isIndexValid(index));
	assert(!cells_[index].frontline);
	cells_[index].frontline = true;

	assert(endFrontline_ < frontline_.size());
	frontline_[endFrontline_] = index;
	endFrontline_ = (endFrontline_ + 1) % frontline_.size();

	if (begFrontline_ == endFrontline_) // need to grow
	{
		std::size_t fSize = frontline_.size();
		frontline_.resize(fSize * 2);

		std::size_t beg, end;
		if (fSize - begFrontline_ < begFrontline_) // end is smaller
		{
			endFrontline_ += fSize;
			beg = 0;
			end = begFrontline_;
		}
		else // beg is smaller
		{
			begFrontline_ += fSize;
			beg = endFrontline_;
			end = fSize;
		}

		// move smallest part
		for (auto i = beg; i < end; ++i)
		{
			frontline_[i + fSize] = std::move(frontline_[i]);
		}
	}
}

std::size_t Board::popFrontline()
{
	assert(!isFrontlineEmpty());
	assert(begFrontline_ < frontline_.size());
	std::size_t index = frontline_[begFrontline_];
	begFrontline_ = (begFrontline_ + 1) % frontline_.size();
	cells_[index].frontline = false;
	return index;
}
