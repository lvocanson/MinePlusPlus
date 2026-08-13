#pragma once

struct NotCopyable
{
	NotCopyable() = default;
	NotCopyable(const NotCopyable&) = delete;
	NotCopyable& operator=(const NotCopyable&) = delete;
};
