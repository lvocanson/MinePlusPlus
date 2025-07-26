#include "Board.h"
#include "Resources.h"
#include <SFML/Graphics.hpp>

class GameScene
{
	static constexpr auto& FONT_REF = Resources::Fonts::roboto;
	static constexpr auto DEFAULT_STRING = "Empty text";
	static constexpr unsigned FONT_SIZE = 20;

public:

	GameScene(Vec2s size, std::size_t mineCount);

	void onWindowResize(sf::Vector2u size);
	void onMouseButtonPressed(sf::Vector2f coordinates, sf::Mouse::Button button);
	void onMouseButtonReleased(sf::Vector2f coordinates, sf::Mouse::Button button);
	void onKeyPressed(sf::Keyboard::Key code);

	void drawOn(sf::RenderTarget& rt) const;

private:

	Board board_;
	sf::Vector2f cellSize_;
	sf::Clock clock_;
	sf::Text left_, center_, right_;
	std::size_t pressedCellIdx_;
	bool gameEnd_;
};
