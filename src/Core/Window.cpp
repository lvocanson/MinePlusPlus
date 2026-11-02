#include "Window.h"
#include "Profiling/Profile.h"
#include <algorithm>
#include <utility>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <imm.h>
#endif

namespace
{

Vec2u getDefaultWindowSize()
{
#ifdef _WIN32
	Vec2f screenSize{float(GetSystemMetrics(SM_CXSCREEN)), float(GetSystemMetrics(SM_CYSCREEN))};
	if (screenSize.x > 0.f && screenSize.y > 0.f)
		return Vec2u(screenSize * 0.75f);
#endif
	// TODO: query the desktop size on other platforms
	return {1280, 720};
}

constexpr std::size_t FRAMEBUFFER_RESERVE_BYTES = std::size_t(7680) * 4320 * sizeof(std::uint32_t);

bool resizeFramebuffer(VirtualRegion& buffer, Vec2i size)
{
	std::size_t bytes = std::size_t(size.x) * std::size_t(size.y) * sizeof(std::uint32_t);
	if (bytes > buffer.getReservedBytes() && !buffer.reserve(std::max(bytes, FRAMEBUFFER_RESERVE_BYTES)))
		return false;
	if (!buffer.commitTo(bytes))
		return false;

	if (bytes < buffer.getCommittedBytes() / 2)
		buffer.decommitTo(bytes);
	return true;
}

} // namespace

Window::~Window()
{
	close();
}

void Window::open(bool fullscreen)
{
	if (window_)
		return;

#ifdef _WIN32
	[[maybe_unused]] static const bool imeDisabled = ImmDisableIME(DWORD(-1));
#endif

	Vec2u size = getDefaultWindowSize();
	unsigned flags = fullscreen
	                 ? MFB_WF_BORDERLESS | MFB_WF_FULLSCREEN_DESKTOP
	                 : MFB_WF_RESIZABLE;
	window_ = mfb_open_ex("Mine++", size.x, size.y, flags);
}

void Window::close()
{
	if (window_)
	{
		mfb_close(window_);
		[[maybe_unused]] auto _ = mfb_update_events(window_);
		window_ = nullptr;
	}
}

// TODO: the new window has none of the callbacks registered on the previous one
bool Window::setFullscreen(bool fullscreen)
{
	if (!window_)
		return false;

	mfb_window* previous = std::exchange(window_, nullptr);
	open(fullscreen);
	if (!window_)
	{
		window_ = previous;
		return false;
	}

	std::swap(window_, previous);
	close();
	window_ = previous;
	return true;
}

bool Window::isOpen() const
{
	return window_ != nullptr;
}

Vec2i Window::getSize() const
{
	if (!window_)
		return {};

	Vec2u size;
	mfb_get_window_size(window_, &size.x, &size.y);
	return Vec2i(size);
}

Canvas Window::beginFrame(Color clearColor)
{
	Vec2i size = getSize();
	if (size != bufferSize_)
	{
		bufferSize_ = resizeFramebuffer(buffer_, size) ? size : Vec2i{};
		MPP_PROFILE_MEMORY("Framebuffer", buffer_.getCommittedBytes());
	}

	Canvas canvas{buffer_.getDataAs<std::uint32_t>(), bufferSize_};
	fill(canvas, clearColor);
	return canvas;
}

void Window::display()
{
	if (!window_)
		return;

	mfb_update_state state = !bufferSize_.x || !bufferSize_.y
	                         ? mfb_update_events(window_)
	                         : mfb_update_ex(window_, buffer_.getData(), unsigned(bufferSize_.x), unsigned(bufferSize_.y));

	if (state == MFB_STATE_EXIT || !mfb_wait_sync(window_))
		window_ = nullptr;
}
