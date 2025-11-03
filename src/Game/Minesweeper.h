#pragma once
#include "Board.h"
#include <SFML/System/Clock.hpp>
#include <SFML/System/Time.hpp>

/*
 * Extends Board with game logic and validation.
 */
class Minesweeper
{
public:

	Minesweeper();

	const Board& getBoard() const { return board_; }
	sf::Time getPlayingTime() const { return clock_.getElapsedTime(); }
	bool isGameOver() const { return state_ == Lost || state_ == Won; }

	void setEasy();
	void setMedium();
	void setHard();

	void resize(const Vec2s& size);
	void setMineCount(std::size_t mineCount);

	void restart();
	void open(const Vec2s& coordinates);
	void flag(const Vec2s& coordinates);

	void setPressedCell(const Vec2s& coordinates) { pressedCell_ = coordinates; }
	const Vec2s& getPressedCell() const { return pressedCell_; }

private:

	Board board_;
	sf::Clock clock_;
	Vec2s pressedCell_;

	enum
	{
		Empty = 0, // The board is not setup
		Ready, // Until first click
		Playing, // After first click until lose or win cond
		Lost,
		Won

	} state_;
};
