#pragma once

// Combines multiple callables into a single object
template <class... Ts>
struct Overloaded : Ts...
{
	using Ts::operator()...;
};
