#include <cstddef>
#include <cstdint>
#include <vector>

struct Cell
{
	bool mined : 1;
	bool opened : 1;
	bool flagged : 1;
	std::uint8_t adjacentMines : 5; // [0, 31]
};

// Function object taking a cell and its index as arguments
template <typename F>
concept CellFunctionObject = requires (F func, Cell c, size_t idx)
{
	{ func(c, idx) };
};

struct Board
{
	Board(std::size_t width, std::size_t height);

	bool areCoordinatesValid(std::size_t col, std::size_t row) const;
	std::size_t toIndex(std::size_t col, std::size_t row) const;

	// call `func` up to 9 times with the cell at (col, row) and its neighbours
	void foreachNeighbour(std::size_t col, std::size_t row, CellFunctionObject auto&& func)
	{
		for (int dr = -1; dr <= 1; ++dr)
		{
			for (int dc = -1; dc <= 1; ++dc)
			{
				const std::size_t nCol = col + dc;
				const std::size_t nRow = row + dr;

				if (!areCoordinatesValid(nCol, nRow))
					continue;

				const auto nIdx = toIndex(nCol, nRow);
				func(cells[nIdx], nIdx);
			}
		}
	}

	// call `func` up to 9 times with the cell at index and its neighbours
	void foreachNeighbour(std::size_t index, CellFunctionObject auto&& func)
	{
		const auto col = index % width;
		const auto row = index / width;
		foreachNeighbour(col, row, std::forward<decltype(func)>(func));
	}

	void addMines(std::size_t count);
	void open(std::size_t index);
	void flag(std::size_t index);

	const std::size_t width, height;
	std::vector<Cell> cells;
};