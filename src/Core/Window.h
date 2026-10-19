#pragma once
#include "Canvas.h"
#include "Utils/NotMovable.h"
#include "Utils/NotCopyable.h"
#include <MiniFB.h>
#include <cstdint>
#include <vector>

class Window : NotCopyable, NotMovable
{
public:

	~Window();

	void open(bool fullscreen);
	void close();
	bool setFullscreen(bool fullscreen);

	bool isOpen() const;
	Vec2i getSize() const;
	mfb_window* getHandle() const { return window_; }

	Canvas beginFrame(Color clearColor);
	void display();

private:

	mfb_window* window_{};
	std::vector<std::uint32_t> buffer_;
	Vec2i bufferSize_{};
};
