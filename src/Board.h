#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

struct Cell
{
	std::uint8_t adjacentMines : 4; // [0, 15]
	bool mined : 1;
	bool opened : 1;
	bool flagged : 1;
	bool frontline : 1;
};

struct Vec2s
{
	std::size_t x, y;

	constexpr Vec2s operator+(const Vec2s&) const;
	constexpr bool operator==(const Vec2s&) const = default;
};
constexpr Vec2s INVALID_VEC2S = {-1, -1};


struct NeighbourRange
{
	NeighbourRange(const class Board& board, const Vec2s& coordinates);

	using nt = std::array<Vec2s, 9>;
	nt neighbours;

	nt::const_iterator begin() const;
	nt::const_iterator end() const;
};

class Board
{
public:

	static Board EasyBoard();
	static Board MediumBoard();
	static Board HardBoard();

public:

	Board();
	Board(const Board&) = default;
	Board(Board&&) noexcept = default;
	Board& operator=(const Board&) = default;
	Board& operator=(Board&&) noexcept = default;

	static bool isSizeValid(const Vec2s& size);
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
	void makeSafe(std::size_t index); // only the index of the last call is guaranted to be safe
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

	void mineCell(std::size_t index);
	bool openCell(Cell& cell);

private:

	Vec2s size_;
	std::size_t mineCount_, flagCount_, openCount_;
	std::vector<Cell> cells_;

private:

	void setupFrontline();
	std::size_t biggestFrontline() const;

	bool isFrontlinePushing() const;
	void pushFrontline(std::size_t index);
	std::size_t popFrontline();

private:

	std::size_t begFrontline_, endFrontline_;
	std::vector<size_t> frontline_;
};
