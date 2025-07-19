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

	void resize(const Vec2s& size);
	const Vec2s& getSize() const { return size_; }
	bool hasValidSize() const { return cells_.size() > 1; }

	std::size_t getMaxNumberOfMines() const { return cells_.size() - 1; }
	void placeMines(std::size_t count, std::size_t safeIndex);
	std::size_t getMineCount() const { return mineCount_; }
	void clear();

	bool areCoordinatesValid(const Vec2s& coordinates) const;
	bool isIndexValid(std::size_t index) const;
	std::size_t toIndex(const Vec2s& coordinates) const;
	Vec2s toCoordinates(std::size_t index) const;

	void open(std::size_t index);
	void flag(std::size_t index);

	const Cell& getCellAt(std::size_t index) const { return cells_[index]; }
	const std::vector<Cell>& getCells() const { return cells_; }
	NeighbourRange getNeigboursOf(const Vec2s& coordinates) const { return {*this, coordinates}; }

private:

	Vec2s size_;
	std::size_t mineCount_, flagCount_;
	std::vector<Cell> cells_;
};
