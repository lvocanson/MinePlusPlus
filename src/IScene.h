#pragma once
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>
#include <memory>

class IScene
{
public:
    virtual ~IScene() = default;

    virtual void onWindowResize(sf::Vector2u newSize) = 0;
    virtual void onMouseButtonPressed(sf::Vector2f position, sf::Mouse::Button button) = 0;
    virtual void onMouseButtonReleased(sf::Vector2f position, sf::Mouse::Button button) = 0;
    virtual void onKeyPressed(sf::Keyboard::Key key) = 0;

    // return scene to transition to, or nullptr
    virtual std::unique_ptr<IScene> update() = 0;
    virtual void drawOn(sf::RenderTarget& target) const = 0;
};
