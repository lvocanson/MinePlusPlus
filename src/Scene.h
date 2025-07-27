#pragma once
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>

template <typename T>
concept Scene = requires (T t, sf::RenderTarget& rt)
{
	{ t.onWindowResize(sf::Vector2u{}) };
	{ t.onMouseButtonPressed(sf::Vector2f{}, sf::Mouse::Button{}) };
	{ t.onMouseButtonReleased(sf::Vector2f{}, sf::Mouse::Button{}) };
	{ t.onKeyPressed(sf::Keyboard::Key{}) };
	{ t.update() };
	{ t.drawOn(rt) };
};
