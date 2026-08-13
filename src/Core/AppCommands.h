#pragma once
#include "AppUI.h"
#include <SFML/Graphics/Color.hpp>
#include <concepts>
#include <functional>
#include <variant>

struct RequestExit {};

struct ChangeClearColor
{
	sf::Color color;
};

struct SwapUI
{
	template <class T>
	static inline auto DEFAULT = [](AppUI& ui) { ui.emplace<T>(); };

	std::function<void(AppUI &)> swapper;
};

using AppCommand = std::variant<
	std::monostate,
	RequestExit,
	ChangeClearColor,
	SwapUI>;

template <class T>
concept ValidCommand = !std::same_as<T, std::monostate> && requires(T t) { AppCommand{t}; };
