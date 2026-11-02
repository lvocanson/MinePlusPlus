#pragma once
#include "Canvas.h"
#include "VirtualRegion.h"
#include "Utils/NotMovable.h"
#include "Utils/NotCopyable.h"
#include <MiniFB.h>
#include <cstdint>

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
	VirtualRegion buffer_;
	Vec2i bufferSize_{};
};
