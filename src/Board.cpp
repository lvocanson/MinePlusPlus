#include "Board.h"
#include <algorithm>
#include <cassert>
#include <random>

Board::Board()
	: size_{}
	, mineCount_{}, flagCount_{}
	, cells_{}
{
}

void Board::resize(const Vec2s& size)
{
	size_ = size;
	clear();
}

void Board::placeMines(std::size_t count, std::size_t safeIndex)
{
	assert(count <= getMaxNumberOfMines());

	if (mineCount_ > 0)
		clear();

	struct It
	{
		using value_type = std::size_t;
		using difference_type = std::ptrdiff_t;
		using iterator_category = std::input_iterator_tag;
		using pointer = const std::size_t*;
		using reference = std::size_t;
		
		value_type value;
		value_type forbidden;


		It& operator++() { if (++value == forbidden) ++value; return *this; }
		bool operator==(const It& other) const = default;
		value_type operator*() const { return value; }
	};

	std::vector<std::size_t> randIndices(count);
	{
		std::mt19937_64 gen(std::random_device{}());
		It first{0, safeIndex}, last{cells_.size(), safeIndex};
		std::sample(first, last, randIndices.begin(), count, gen);
	}

	for (std::size_t index : randIndices)
	{
		cells_[index].mined = true;
		for (auto coordinates : getNeigboursOf(toCoordinates(index)))
		{
			++cells_[toIndex(coordinates)].adjacentMines;
		}
	}
	mineCount_ = count;
}

void Board::clear()
{
	mineCount_ = flagCount_ = 0;
	cells_.assign(size_.x * size_.y, {});
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

void Board::open(std::size_t index)
{
	assert(isIndexValid(index));

	auto& first = cells_[index];
	if (first.flagged)
		return;

	std::vector<std::size_t> cellIndices;

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
			else if (!c.opened) cellIndices.emplace_back(idx);
		};

		if (flaggedNeighbourCount != first.adjacentMines)
			return;
	}
	else
	{
		cellIndices = {index};
	}

	while (!cellIndices.empty())
	{
		index = cellIndices.back();
		cellIndices.pop_back();

		auto& cell = cells_[index];
		cell.opened = true;
		cell.flagged = false;

		if (cell.adjacentMines == 0)
		{
			for (auto coordinates : getNeigboursOf(toCoordinates(index)))
			{
				auto idx = toIndex(coordinates);
				if (!cells_[idx].opened)
					cellIndices.emplace_back(idx);
			};
		}
	}
}

void Board::flag(std::size_t index)
{
	assert(isIndexValid(index));
	auto& cell = cells_[index];
	if (!cell.opened)
		cell.flagged ^= true;
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
