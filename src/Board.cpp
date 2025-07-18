#include "Board.h"
#include <cassert>
#include <random>


Board::Board(std::size_t width, std::size_t height)
	: width(width), height(height)
	, cells(width* height, Cell{})
{
	assert(cells.size() > 0 && "width and height must be non-zero numbers");
}

bool Board::areCoordinatesValid(std::size_t col, std::size_t row) const
{
	// coordinates are unsigned, no need to check >= 0
	return col < width && row < height;
}

std::size_t Board::toIndex(std::size_t col, std::size_t row) const
{
	assert(areCoordinatesValid(col, row));
	return col + row * width;
}

void Board::addMines(std::size_t count)
{
	std::mt19937_64 gen(std::random_device{}());
	std::uniform_int_distribution<std::size_t> dist(0, cells.size() - 1);

	for (std::size_t minesRemaining = count; minesRemaining > 0;)
	{
		const auto randIdx = dist(gen);
		auto& randCell = cells[randIdx];
		if (randCell.mined)
			continue;

		randCell.mined = true;
		foreachNeighbour(randIdx, [](Cell& c, auto) { ++c.adjacentMines; });
		--minesRemaining;
	}
}

void Board::open(std::size_t index)
{
	assert(index < cells.size());

	auto& first = cells[index];
	if (first.flagged)
		return;

	std::vector<std::size_t> cellIndices;

	if (first.opened)
	{
		std::size_t flaggedNeighbourCount = 0;
		foreachNeighbour(index, [&](Cell& c, size_t i)
		{
			if (c.flagged) ++flaggedNeighbourCount;
		});

		if (flaggedNeighbourCount == first.adjacentMines)
		{
			foreachNeighbour(index, [&](Cell& c, size_t i)
			{
				if (!c.opened && !c.flagged) cellIndices.emplace_back(i);
			});
		}
	}
	else
	{
		cellIndices = {index};
	}

	while (!cellIndices.empty())
	{
		index = cellIndices.back();
		cellIndices.pop_back();

		auto& cell = cells[index];
		cell.opened = true;
		cell.flagged = false;
		if (cell.adjacentMines == 0)
		{
			foreachNeighbour(index, [&](Cell& c, size_t i)
			{
				if (!c.opened) cellIndices.emplace_back(i);
			});
		}
	}
}

void Board::flag(std::size_t index)
{
	assert(index < cells.size());
	auto& cell = cells[index];
	if (!cell.opened)
		cell.flagged ^= true;
}
