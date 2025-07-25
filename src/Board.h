#include <cstddef>
#include <cstdint>
#include <vector>

struct Cell
{
	std::uint8_t adjacentMines : 4; // [0, 15]
	bool watched : 1; // flag for open algo
	bool mined : 1;
	bool opened : 1;
	bool flagged : 1;
};

struct Vec2s
{
	std::size_t x, y;
};

class Board;

struct NeighbourRange
{
	NeighbourRange(const Board& board, const Vec2s& coordinates);

	struct Iterator
	{
		Iterator() = default;
		Iterator(const NeighbourRange* range);
		Iterator& operator++();
		bool operator==(const Iterator& other) const { return range == other.range; }
		Vec2s operator*() const;

	private:

		const NeighbourRange* range = nullptr;
		int dc, dr;
	};

	Iterator begin() const { return this; }
	Iterator end() const { return {}; }

private:

	const Board& board;
	const Vec2s coordinates;
};

class Board
{
public:

	Board();

	bool isSizeValid(const Vec2s& size) const;
	void resize(const Vec2s& size);
	const Vec2s& getSize() const { return size_; }

	bool areCoordinatesValid(const Vec2s& coordinates) const;
	bool isIndexValid(std::size_t index) const;
	std::size_t toIndex(const Vec2s& coordinates) const;
	Vec2s toCoordinates(std::size_t index) const;

	std::size_t getMaxNumberOfMines() const { return cells_.size() - 1; }
	void setMineCount(std::size_t mineCount);
	std::size_t getMineCount() const { return mineCount_; }

	void placeMines();
	void clear();

	// returns true if mine opened
	bool open(std::size_t index);
	std::size_t getOpenCount() const { return openCount_; }

	void flag(std::size_t index);
	std::size_t getFlagCount() const { return flagCount_; }

	bool isWon() const { return openCount_ == cells_.size() - mineCount_; }

	const Cell& getCellAt(std::size_t index) const { return cells_[index]; }
	const std::vector<Cell>& getCells() const { return cells_; }
	NeighbourRange getNeigboursOf(const Vec2s& coordinates) const { return {*this, coordinates}; }

private:

	bool openCell(Cell& cell);
	void watch(std::size_t index);
	std::size_t popLastWatched();

	Vec2s size_;
	std::size_t mineCount_, flagCount_, openCount_;
	std::vector<Cell> cells_;
	std::vector<std::size_t> watchedIndices_;

public:

	std::size_t noAdj = 0;
	std::size_t initWatched = 0;
	std::size_t maxWatched = 0;
};
