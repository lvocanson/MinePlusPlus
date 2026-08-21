#include "Board.h"
#include "Utils/MyRandom.h"
#include "Utils/Overloaded.h"
#include <algorithm>
#include <cassert>
#include <limits>
#include <utility>
#include <variant>

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
	, cells_{}
{}

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

struct Board::SeedStack
{
#ifdef MPP_BOARD_FIXED_SEED_STACK_CAPACITY
	static constexpr std::size_t CAPACITY = MPP_BOARD_FIXED_SEED_STACK_CAPACITY;
#else
	static constexpr std::size_t CAPACITY = 32;
#endif // MPP_BOARD_FIXED_SEED_STACK_CAPACITY

	struct Fixed { std::size_t buf[CAPACITY]; std::size_t top = 0; };
	struct Heap { std::vector<std::size_t> vec; };

	std::variant<Fixed, Heap> store;

	bool empty() const
	{
		return std::visit(Overloaded
			{
				[](const Fixed& f) { return f.top == 0; },
				[](const Heap& h) { return h.vec.empty(); }
			}, store);
	}

	std::size_t pop()
	{
		return std::visit(Overloaded
			{
				[](Fixed& f) { return f.buf[--f.top]; },
				[](Heap& h) { std::size_t i = h.vec.back(); h.vec.pop_back(); return i; }
			}, store);
	}

	void push(std::size_t index)
	{
		std::visit(Overloaded
			{
				[&](Fixed& f)
				{
					if (f.top < CAPACITY)
					{
						f.buf[f.top++] = index;
						return;
					}
					// Overflow: switch to the heap alternative, carrying the buffered seeds.
					Heap h;
					h.vec.reserve(CAPACITY * 2);
					h.vec.assign(f.buf, f.buf + f.top);
					h.vec.push_back(index);
					store.emplace<Heap>(std::move(h));
				},
				[&](Heap& h) { h.vec.push_back(index); }
			}, store);
	}
};

bool Board::open(std::size_t index)
{
	assert(isIndexValid(index));

	auto& first = cells_[index];
	if (first.flagged)
		return false;

	bool mineOpened = false;
	SeedStack stack;

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
			flaggedNeighbourCount += cells_[toIndex(coo)].flagged;
		}

		if (flaggedNeighbourCount != first.adjacentMines)
			return false;

		chord(coordinates, stack, mineOpened);
	}

	while (!stack.empty())
	{
		fillFrom(stack.pop(), stack, mineOpened);
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
	++openCount_;
	return cell.mined;
}

void Board::chord(const Vec2s& cursor, SeedStack& stack, bool& mineOpened)
{
	for (auto& coo : getNeighboursOf(cursor))
	{
		std::size_t index = toIndex(coo);
		auto& cell = cells_[index];

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
	auto& seed = cells_[index];
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
		auto& cell = cells_[l - 1];
		if (cell.opened)
			break;

		mineOpened |= openCell(cell);
		if (cell.adjacentMines)
			break;

		--l;
	}

	while (r < rEnd)
	{
		auto& cell = cells_[r + 1];
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
	if (index + width < cells_.size())
		scanRow(a + width, b + width, stack, mineOpened);
}

void Board::scanRow(std::size_t l, std::size_t r, SeedStack& stack, bool& mineOpened)
{
	for (std::size_t i = l; i <= r; ++i)
	{
		auto& cell = cells_[i];
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
			auto next = cells_[i + 1];
			if (next.opened || next.adjacentMines)
				break;

			// skip the remaining non-opened cells
			// will be opened on the next fillFrom
			++i;
		}
	}
}
