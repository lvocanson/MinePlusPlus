#include "MenuScene.h"
#include "GameScene.h"

void MenuScene::onWindowResize(sf::Vector2u size)
{
}

void MenuScene::onMouseButtonPressed(sf::Vector2f coordinates, sf::Mouse::Button button)
{
}

void MenuScene::onMouseButtonReleased(sf::Vector2f coordinates, sf::Mouse::Button button)
{
}

void MenuScene::onKeyPressed(sf::Keyboard::Key code)
{
}

std::unique_ptr<IScene> MenuScene::update()
{
	nextScene_ = std::make_unique<GameScene>(Vec2s{16, 16}, 40);
	return std::move(nextScene_);
}

void MenuScene::drawOn(sf::RenderTarget& rt) const
{
}
