#pragma once
#include "IScene.h"

class MenuScene : public IScene
{
	void onWindowResize(sf::Vector2u size) override;
	void onMouseButtonPressed(sf::Vector2f coordinates, sf::Mouse::Button button) override;
	void onMouseButtonReleased(sf::Vector2f coordinates, sf::Mouse::Button button) override;
	void onKeyPressed(sf::Keyboard::Key code) override;

	std::unique_ptr<IScene> update() override;
	void drawOn(sf::RenderTarget& rt) const override;

private:

	std::unique_ptr<IScene> nextScene_;

};
