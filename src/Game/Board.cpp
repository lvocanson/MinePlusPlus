#include "Board.h"
#include "Utils/CircularBuffer.h"
#include "Utils/MyRandom.h"
#include <algorithm>
#include <cassert>
#include <limits>

namespace
{

// Adds a signed offset to an unsigned coordinate. Returns false on under/overflow.
constexpr bool tryAddDelta(std::size_t value, std::ptrdiff_t delta, std::size_t& result)
{
	// Unsigned negation is well defined, unlike -PTRDIFF_MIN
	std::size_t magnitude = delta < 0 ? std::size_t{0} - std::size_t(delta) : std::size_t(delta);

	if (delta < 0)
	{
		if (value < magnitude)
			return false;
		result = value - magnitude;
	}
	else
	{
		if (std::numeric_limits<std::size_t>::max() - value < magnitude)
			return false;
		result = value + magnitude;
	}
	return true;
}

constexpr std::array<Vec2sDelta, 9> dcoordinates
{
	{
		{-1, -1},
		{0, -1},
		{1, -1},
		{-1, 0},
		{0, 0},
		{1, 0},
		{-1, 1},
		{0, 1},
		{1, 1}
	}
};

} // namespace

constexpr std::optional<Vec2s> Vec2s::operator+(const Vec2sDelta& rhs) const
{
	Vec2s result;
	if (!tryAddDelta(x, rhs.x, result.x) || !tryAddDelta(y, rhs.y, result.y))
		return std::nullopt;
	return result;
}

NeighbourRange::NeighbourRange(const Board& board, const Vec2s& coordinates)
	: count(0)
{
	for (auto& dc : dcoordinates)
	{
		auto coo = coordinates + dc;
		if (coo && board.areCoordinatesValid(*coo))
			neighbours[count++] = *coo;
	}
}

NeighbourRange::nt::iterator NeighbourRange::begin()
{
	return neighbours.begin();
}

NeighbourRange::nt::const_iterator NeighbourRange::begin() const
{
	return neighbours.begin();
}

NeighbourRange::nt::iterator NeighbourRange::end()
{
	return neighbours.begin() + count;
}

NeighbourRange::nt::const_iterator NeighbourRange::end() const
{
	return neighbours.begin() + count;
}

Board::Board()
	: size_{}
	, mineCount_{}
	, flagCount_{}
	, openCount_{}
	, cells_{} {}

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

	// Fisher-Yates shuffle variant
	for (std::size_t i = cells_.size() - mineCount_; i < cells_.size(); ++i)
	{
		std::uniform_int_distribution<std::size_t> dist(0, i);
		std::size_t r = dist(gen);
		std::size_t index = cells_[r].mined ? i : r;
		mineCell(index);
	}
}

void Board::clear()
{
	assert(isSizeValid(size_));
	flagCount_ = openCount_ = 0;
	cells_.assign(size_.x * size_.y, {});
}

void Board::makeSafe(std::size_t index)
{
	assert(isIndexValid(index));

	auto& cell = cells_[index];
	if (!cell.mined)
		return;

	// mine the n-th not already mined cell
	std::size_t spotsLeft = cells_.size() - mineCount_;
	std::uniform_int_distribution<std::size_t> dist(1, spotsLeft);
	std::size_t n = dist(gen);
	for (std::size_t i = 0; i < cells_.size(); ++i)
	{
		if (!cells_[i].mined && --n == 0)
			mineCell(i);
	}

	clearCell(index);
}

std::size_t Board::moveMine(std::size_t index)
{
	assert(isIndexValid(index));

	auto& cell = cells_[index];
	if (!cell.mined)
		return index;

	std::size_t unoccupiedNbCount = 0;
	std::array<std::size_t, 8> unoccupiedNbIndexes;
	auto cellNeighbours = getNeighboursOf(toCoordinates(index));
	for (auto& coordinates : cellNeighbours)
	{
		// can't move to if its mined or opened
		std::size_t idx = toIndex(coordinates);
		if (cells_[idx].mined || cells_[idx].opened)
			continue;

		unoccupiedNbIndexes[unoccupiedNbCount] = idx;
		++unoccupiedNbCount;
	}

	if (unoccupiedNbCount == 0)
		return index;

	clearCell(index);
	std::uniform_int_distribution<std::size_t> dist(0, unoccupiedNbCount - 1);
	std::size_t idx = unoccupiedNbIndexes[dist(gen)];
	mineCell(idx);

	return idx;
}

struct Board::OpenImpl
{
	static constexpr std::size_t FRONTLINE_SIZE = 20;

	// Frontline buffer used for BFS expansion.
	// If it fills up (large open area with few mines), we fall back to a full scan.
	CircularBuffer<std::size_t, FRONTLINE_SIZE> frontline;

	// If true, indicates that the frontline was full when we needed to push a cell
	// to it. We then need to iterate over all cell to retrieve this missed cell.
	bool needFullScan;

	bool mineOpened;

	bool hasWork() const { return !frontline.isEmpty() || needFullScan; };
	void pushToFrontline(std::size_t idx) { needFullScan |= !frontline.tryPush(idx); }
};

bool Board::open(std::size_t index)
{
	assert(isIndexValid(index));

	auto& first = cells_[index];
	if (first.flagged)
		return false;

	OpenImpl impl{};

	if (!first.opened)
	{
		// Opening a fresh cell:
		// - If it's a mine or has adjacent mines, handle it directly.
		// - Otherwise, start BFS expansion.

		if (openCell(first))
			return true;

		if (first.adjacentMines)
			return false;

		auto neighbours = getNeighboursOf(toCoordinates(index));
		openOrPush(neighbours, impl);
	}
	else
	{
		// Re-opening an already opened cell (safe chording):
		// Only open neighbours if the number of flagged cells
		// matches the number of adjacent mines.
		std::size_t flaggedNeighbourCount = 0;

		auto neighbours = getNeighboursOf(toCoordinates(index));
		auto newEnd = neighbours.begin();

		// Compact the non flagged neighbours in place. newEnd never runs
		// ahead of the read cursor, so no entry is overwritten before use.
		for (auto& coo : neighbours)
		{
			if (cells_[toIndex(coo)].flagged)
				++flaggedNeighbourCount;
			else
				*newEnd++ = coo;
		}

		if (flaggedNeighbourCount != first.adjacentMines)
			return false;

		neighbours.count = std::size_t(newEnd - neighbours.begin());
		openOrPush(neighbours, impl);
	}

	// BFS expansion phase:
	// Repeatedly open safe neighbours from the frontline.
	// Fulls scan is needed if some cells were marked as frontline but
	// could not be pushed into the buffer due to its capacity limit.
	// The full scan finds those missed cells and continues the expansion.
	while (impl.hasWork())
	{
		computeFrontline(impl);
		if (impl.needFullScan)
			fullScan(impl);
	}

	return impl.mineOpened;
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
	for (auto& coordinates : getNeighboursOf(toCoordinates(index)))
	{
		++cells_[toIndex(coordinates)].adjacentMines;
	}
}

void Board::clearCell(std::size_t index)
{
	assert(isIndexValid(index));
	assert(cells_[index].mined);
	cells_[index].mined = false;
	for (auto& coordinates : getNeighboursOf(toCoordinates(index)))
	{
		--cells_[toIndex(coordinates)].adjacentMines;
	}
}

bool Board::openCell(Cell& cell)
{
	assert(!cell.opened);
	cell.opened = true;
	flagCount_ -= cell.flagged;
	cell.flagged = false;
	cell.frontline = false;
	++openCount_;
	return cell.mined;
}

// Open every unopened neighbour with adjacent mines.
// Push every unopened neigbours without adajcent mines.
void Board::openOrPush(NeighbourRange& neighbours, OpenImpl& impl)
{
	for (auto& coo : neighbours)
	{
		std::size_t index = toIndex(coo);
		auto& cell = cells_[index];

		if (cell.opened)
			continue;

		if (cell.adjacentMines)
		{
			impl.mineOpened |= openCell(cell);
		}
		else if (!cell.frontline)
		{
			cell.frontline = true;
			impl.pushToFrontline(index);
		}
	}
}

// Core BFS algorithm.
void Board::computeFrontline(OpenImpl& impl)
{
	while (!impl.frontline.isEmpty())
	{
		std::size_t index = impl.frontline.pop();
		auto& cell = cells_[index];
		assert(!cell.opened && cell.frontline);

		openCell(cell);
		auto neighbours = getNeighboursOf(toCoordinates(index));
		openOrPush(neighbours, impl);
	}
}

// Iterate over all cells and check for missed frontline cells.
// Needed if the frontline buffer overflows.
void Board::fullScan(OpenImpl& impl)
{
	impl.needFullScan = false;
	for (std::size_t i = 0; i < cells_.size(); ++i)
	{
		auto& cell = cells_[i];
		if (cell.frontline)
		{
			openCell(cell);
			cleanFrontline(impl);

			auto neighbours = getNeighboursOf(toCoordinates(i));
			openOrPush(neighbours, impl);
		}
	}
}

// Remove invalid frontline cells from the frontline.
// Needed if we want to open while full scanning.
void Board::cleanFrontline(OpenImpl& impl)
{
	std::size_t size = impl.frontline.size();
	for (std::size_t i = 0; i < size; ++i)
	{
		std::size_t index = impl.frontline.pop();
		auto& cell = cells_[index];

		if (cell.frontline && !cell.opened)
		{
			impl.frontline.push(index);
		}
	}
}
