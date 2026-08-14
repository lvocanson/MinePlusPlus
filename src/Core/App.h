#pragma once
#include "AppCommands.h"
#include "Audio.h"
#include "Game/Minesweeper.h"
#include "Utils/NotCopyable.h"
#include "Utils/NotMovable.h"
#include <SFML/Graphics/RenderWindow.hpp>
#include <array>
#include <cassert>
#include <exception>

class App : NotCopyable, NotMovable
{
public:

	App();
	int run();

public:

	Minesweeper& getGame() { return game_; }
	Audio& getAudio() { return audio_; }

public:

	// Defer execution until end of frame
	template <ValidCommand T, class... Args>
	T& submitCommand(Args&&... args)
	{
		for (auto& command : commands_)
			if (command.index() == 0)
				return command.emplace<T>(std::forward<Args>(args)...);

		assert(false && "Command queue is too small");
		std::terminate();
	}

public:

	void resetView();
	void centerView(sf::FloatRect target);

private:

	void pollEvents();
	void processCommands();

private:

	sf::RenderWindow window_;
	sf::Color clearColor_;
	bool isMouseDraggingCamera_;
	Minesweeper game_;
	AppUI ui_;
	Audio audio_;
	std::array<AppCommand, 2> commands_;
};
