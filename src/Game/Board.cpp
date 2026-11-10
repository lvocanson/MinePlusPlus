#include "Board.h"
#include "Profiling/Profile.h"
#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <utility>

namespace
{

void countAdjacentMines(Cell* data, const Vec2s& size)
{
	std::size_t width = size.x;
	for (std::size_t y = 0; y < size.y; ++y)
	{
		Cell* row = data + y * width;
		unsigned hasUp = y > 0, hasDown = y + 1 < size.y;
		const Cell* up = hasUp ? row - width : row;
		const Cell* down = hasDown ? row + width : row;
		auto column = [&](std::size_t x) { return row[x].mined + hasUp * up[x].mined + hasDown * down[x].mined; };

		unsigned previous = 0, current = column(0);
		for (std::size_t x = 0; x < width; ++x)
		{
			unsigned next = x + 1 < width ? column(x + 1) : 0;
			row[x].adjacentMines = std::uint8_t(previous + current + next);
			previous = current;
			current = next;
		}
	}
}

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

using SeedIndex = std::uint32_t;
static_assert(Board::MAX_CELLS <= std::numeric_limits<SeedIndex>::max());

constexpr std::size_t getMaxSeedBytes(std::size_t cellCount)
{
	constexpr std::size_t maxCellCount = (std::numeric_limits<std::size_t>::max() / sizeof(SeedIndex) - 8) / 4;
	return cellCount <= maxCellCount
	       ? (4 * cellCount + 8) * sizeof(SeedIndex)
	       : std::numeric_limits<std::size_t>::max();
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

constexpr std::optional<Vec2s> operator+(const Vec2s& lhs, const Vec2sDelta& rhs)
{
	Vec2s result;
	if (!tryAddDelta(lhs.x, rhs.x, result.x) || !tryAddDelta(lhs.y, rhs.y, result.y))
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
{}

bool Board::isSizeValid(const Vec2s& size)
{
	return size.x && size.y && size.x <= MAX_CELLS / size.y;
}

void Board::resize(const Vec2s& size)
{
	assert(isSizeValid(size));
	size_ = size;
	flagCount_ = openCount_ = 0;
	std::size_t cellBytes = getCellCount() * sizeof(Cell);
	// TODO: no recovery when the address space cannot be reserved (huge boards on 32-bit builds)
	if (!cellRegion_.reserve(cellBytes) || !seedRegion_.reserve(getMaxSeedBytes(getCellCount())))
		std::abort();
	// TODO: no recovery when the OS refuses to commit
	if (!cellRegion_.commitTo(cellBytes))
		std::abort();

	// Don't need to clear(), freshly committed pages are zeroed.
	MPP_PROFILE_MEMORY("Board cells", cellRegion_.getCommittedBytes());
}

bool Board::areCoordinatesValid(const Vec2s& coordinates) const
{
	// coordinates are unsigned, no need to check >= 0
	return coordinates.x < size_.x && coordinates.y < size_.y;
}

bool Board::isIndexValid(std::size_t index) const
{
	return index < getCellCount();
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

void Board::placeMines(std::uint32_t seed)
{
	assert(isSizeValid(size_));
	random_ = {seed};

	// Floyd's sampling algorithm (Bentley & Floyd, 1987)
	for (std::size_t i = getCellCount() - mineCount_; i < getCellCount(); ++i)
	{
		std::size_t r = random_.nextBelow(std::uint32_t(i + 1));
		std::size_t index = cells()[r].mined ? i : r;
		cells()[index].mined = true;
	}

	countAdjacentMines(cells().data(), size_);
}

void Board::clear()
{
	assert(isSizeValid(size_));
	flagCount_ = openCount_ = 0;
	seedRegion_.decommitTo(0);
	std::memset(cellRegion_.getData(), 0, getCellCount() * sizeof(Cell));
}

void Board::makeSafe(std::size_t index)
{
	assert(isIndexValid(index));

	auto& cell = cells()[index];
	if (!cell.mined)
		return;

	// mine the n-th not already mined cell
	std::size_t spotsLeft = getCellCount() - mineCount_;
	std::size_t n = 1 + random_.nextBelow(std::uint32_t(spotsLeft));
	for (std::size_t i = 0; i < getCellCount(); ++i)
	{
		if (!cells()[i].mined && --n == 0)
		{
			mineCell(i);
			break;
		}
	}

	clearCell(index);
}

std::size_t Board::moveMine(std::size_t index)
{
	assert(isIndexValid(index));

	auto& cell = cells()[index];
	if (!cell.mined)
		return index;

	std::size_t unoccupiedNbCount = 0;
	std::array<std::size_t, 8> unoccupiedNbIndexes;
	auto cellNeighbours = getNeighboursOf(toCoordinates(index));
	for (auto& coordinates : cellNeighbours)
	{
		// can't move to if its mined or opened
		std::size_t idx = toIndex(coordinates);
		if (cells()[idx].mined || cells()[idx].opened)
			continue;

		unoccupiedNbIndexes[unoccupiedNbCount] = idx;
		++unoccupiedNbCount;
	}

	if (unoccupiedNbCount == 0)
		return index;

	clearCell(index);
	std::size_t idx = unoccupiedNbIndexes[random_.nextBelow(std::uint32_t(unoccupiedNbCount))];
	mineCell(idx);

	return idx;
}

struct Board::SeedStack
{
	VirtualRegion& region;
	std::size_t top = 0;

	bool empty() const { return top == 0; }

	std::size_t pop() { return region.getDataAs<SeedIndex>()[--top]; }

	void push(std::size_t index)
	{
		std::size_t bytes = (top + 1) * sizeof(SeedIndex);
		if (bytes > region.getCommittedBytes())
		{
			assert(bytes <= region.getReservedBytes());
			std::size_t target = std::min(std::max(bytes, region.getCommittedBytes() * 2), region.getReservedBytes());
			// TODO: no recovery when the OS refuses to commit
			if (!region.commitTo(target))
				std::abort();
		}
		region.getDataAs<SeedIndex>()[top++] = SeedIndex(index);
	}
};

bool Board::open(std::size_t index)
{
	assert(isIndexValid(index));

	auto& first = cells()[index];
	if (first.flagged)
		return false;

	bool mineOpened = false;
	SeedStack stack{seedRegion_};

	if (!first.opened)
	{
		if (first.mined)
		{
			openCell(first);
			return true;
		}

		if (first.adjacentMines)
		{
			openCell(first);
			return false;
		}

		stack.push(index);
	}
	else
	{
		// Chording: only expand if the flag count matches
		auto coordinates = toCoordinates(index);
		std::size_t flaggedNeighbourCount = 0;
		for (auto& coo : getNeighboursOf(coordinates))
		{
			flaggedNeighbourCount += cells()[toIndex(coo)].flagged;
		}

		if (flaggedNeighbourCount != first.adjacentMines)
			return false;

		chord(coordinates, stack, mineOpened);
	}

	while (!stack.empty())
	{
		fillFrom(stack.pop(), stack, mineOpened);
	}

	MPP_PROFILE_MEMORY_PEAK("Seed stack", seedRegion_.getCommittedBytes());
	return mineOpened;
}

void Board::flag(std::size_t index)
{
	assert(isIndexValid(index));
	auto& cell = cells()[index];
	if (!cell.opened)
	{
		flagCount_ += std::size_t(cell.flagged ^= true) * 2 - 1;
	}
}

#ifdef MPP_CHEATS
void Board::revealNumbers()
{
	for (auto& cell : cells())
		if (!cell.opened && !cell.mined && cell.adjacentMines)
			openCell(cell);
}

void Board::flagMinesOnly()
{
	flagCount_ = 0;
	for (auto& cell : cells())
	{
		if (!cell.opened)
		{
			cell.flagged = cell.mined;
			flagCount_ += cell.flagged;
		}
	}
}
#endif

void Board::mineCell(std::size_t index)
{
	assert(isIndexValid(index));
	assert(!cells()[index].mined);
	cells()[index].mined = true;
	for (auto& coordinates : getNeighboursOf(toCoordinates(index)))
	{
		++cells()[toIndex(coordinates)].adjacentMines;
	}
}

void Board::clearCell(std::size_t index)
{
	assert(isIndexValid(index));
	assert(cells()[index].mined);
	cells()[index].mined = false;
	for (auto& coordinates : getNeighboursOf(toCoordinates(index)))
	{
		--cells()[toIndex(coordinates)].adjacentMines;
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

void Board::chord(const Vec2s& cursor, SeedStack& stack, bool& mineOpened)
{
	for (auto& coo : getNeighboursOf(cursor))
	{
		std::size_t index = toIndex(coo);
		auto& cell = cells()[index];

		if (cell.opened || cell.flagged)
			continue;

		if (cell.adjacentMines)
			mineOpened |= openCell(cell);
		else
			stack.push(index);
	}
}

void Board::fillFrom(std::size_t index, SeedStack& stack, bool& mineOpened)
{
	auto& seed = cells()[index];
	if (seed.opened || seed.adjacentMines)
		return;

	// Row limits
	std::size_t width = size_.x;
	std::size_t rBeg = index / width * width;
	std::size_t rEnd = rBeg + width - 1;

	// Open row (left and right) until numbers
	mineOpened |= openCell(seed);
	std::size_t l = index, r = index;

	while (l > rBeg)
	{
		auto& cell = cells()[l - 1];
		if (cell.opened)
			break;

		mineOpened |= openCell(cell);
		if (cell.adjacentMines)
			break;

		--l;
	}

	while (r < rEnd)
	{
		auto& cell = cells()[r + 1];
		if (cell.opened)
			break;

		mineOpened |= openCell(cell);
		if (cell.adjacentMines)
			break;

		++r;
	}

	std::size_t a = (l > rBeg) ? l - 1 : l;
	std::size_t b = (r < rEnd) ? r + 1 : r;

	if (index >= width)
		scanRow(a - width, b - width, stack, mineOpened);
	if (index + width < getCellCount())
		scanRow(a + width, b + width, stack, mineOpened);
}

void Board::scanRow(std::size_t l, std::size_t r, SeedStack& stack, bool& mineOpened)
{
	for (std::size_t i = l; i <= r; ++i)
	{
		auto& cell = cells()[i];
		if (cell.opened)
			continue;

		if (cell.adjacentMines)
		{
			// Can be opened now without recursion
			mineOpened |= openCell(cell);
			continue;
		}

		stack.push(i);
		while (i < r)
		{
			auto next = cells()[i + 1];
			if (next.opened || next.adjacentMines)
				break;

			// skip the remaining non-opened cells
			// will be opened on the next fillFrom
			++i;
		}
	}
}
