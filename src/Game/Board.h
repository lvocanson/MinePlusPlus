#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

struct Cell
{
	std::uint8_t adjacentMines : 4; // [0, 15]
	bool mined                 : 1;
	bool opened                : 1;
	bool flagged               : 1;
	bool frontline             : 1;
};

// Signed offset between two Vec2s.
struct Vec2sDelta
{
	std::ptrdiff_t x, y;
};

struct Vec2s
{
	std::size_t x, y;

	// Returns nullopt if the result is not representable as a Vec2s.
	constexpr std::optional<Vec2s> operator+(const Vec2sDelta&) const;
	constexpr bool operator==(const Vec2s&) const = default;
};
constexpr Vec2s INVALID_VEC2S = {std::size_t(-1), std::size_t(-1)};


struct NeighbourRange
{
	NeighbourRange(const class Board& board, const Vec2s& coordinates);

	using nt = std::array<Vec2s, 9>;
	nt neighbours;
	std::size_t count;

	nt::iterator begin();
	nt::const_iterator begin() const;
	nt::iterator end();
	nt::const_iterator end() const;
};

/*
 * Simple container for cells.
 * This class has no knowledge of game logic.
 * Assumes proper usage: no validation is performed.
 */
class Board
{
public:

	Board();
	Board(const Board&) = default;
	Board(Board&&) noexcept = default;
	Board& operator=(const Board&) = default;
	Board& operator=(Board&&) noexcept = default;

public: // setup methods

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
	void clear();

	// Make sure the 'index' cell is not mined, moving the mine to an other random
	// cell. Only the last index passed to this function is guarenteed safe.
	void makeSafe(std::size_t index);

	// Move the mine at 'index' to a neighbour and returns its new index.
	// The returned index can be the same as 'index' if the method failed.
	std::size_t moveMine(std::size_t index);

public: // playing methods

	// returns true if mine opened
	bool open(std::size_t index);
	std::size_t getOpenCount() const { return openCount_; }

	void flag(std::size_t index);
	std::size_t getFlagCount() const { return flagCount_; }

	bool isWon() const { return openCount_ == cells_.size() - mineCount_; }

	const Cell& getCellAt(std::size_t index) const { return cells_[index]; }
	const std::vector<Cell>& getCells() const { return cells_; }
	NeighbourRange getNeighboursOf(const Vec2s& coordinates) const { return {*this, coordinates}; }

private: // setup helpers

	void mineCell(std::size_t index);
	void clearCell(std::size_t index);

private: // open helpers

	struct OpenImpl;
	bool openCell(Cell& cell);
	void openOrPush(NeighbourRange& neighbours, OpenImpl& impl);
	void computeFrontline(OpenImpl& impl);
	void fullScan(OpenImpl& impl);
	void cleanFrontline(OpenImpl& impl);

private:

	Vec2s size_;
	std::size_t mineCount_, flagCount_, openCount_;
	std::vector<Cell> cells_;
};
