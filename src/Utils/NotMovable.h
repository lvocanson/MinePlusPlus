#pragma once

struct NotMovable
{
	NotMovable() = default;
	NotMovable(NotMovable&&) = delete;
	NotMovable& operator=(NotMovable&&) = delete;
};
