#pragma once
#include "Board.h"
#include "BoardRenderer.h"
#include "GameControls.h"
#include "Core/Canvas.h"
#include "Utils/Stopwatch.h"

struct View;

class Minesweeper
{
public:

	Minesweeper();

	const Board& getBoard() const { return board_; }
	Stopwatch::Clock::duration getPlayingTime() const { return clock_.getElapsedTime(); }
	bool isGameOver() const { return state_ == Lost || state_ == Won; }
	bool isLost() const { return state_ == Lost; }

	void setEasy();
	void setMedium();
	void setHard();

	void resize(const Vec2s& size);
	void setMineCount(std::size_t mineCount);

	void restart();
	void open(const Vec2s& coordinates);
	void flag(const Vec2s& coordinates);

public:

	void dispatchWorldEvent(const WorldEvent& event);
	void update(View& view, float dt);
	void render(Canvas canvas, const View& view) const;

public:

	void resetParameters();

	void setPressedCell(std::optional<Vec2s> coordinates);
	std::optional<Vec2s> getPressedCell() const { return pressedCell_; }
	void setRendering(bool active) { rendering_ = active; }

	void setRotationSpeed(float speed) { rotationSpeed_ = speed; }
	float getRotationSpeed() const { return rotationSpeed_; }
	void setRunningBombCount(std::size_t count);
	std::size_t getRunningBombCount() const { return runningBombCount_; }

private:

	void randomizeRunningBombIndexes();

private:

	Board board_;
	Stopwatch clock_;
	std::optional<Vec2s> pressedCell_;
	GameControls controls_;
	bool rendering_;

	enum : char
	{
		Empty = 0, // The board is not setup
		Ready,     // Until first click
		Playing,   // After first click until lose or win cond
		Lost,
		Won

	} state_;

	float rotationSpeed_;

	// Requested count (game mode parameter), and the indexes of the
	// running bombs, only filled while playing.
	std::size_t runningBombCount_;
	std::vector<std::size_t> runningBombIndexes_;
};
