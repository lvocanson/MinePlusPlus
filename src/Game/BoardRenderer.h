#pragma once
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/Graphics/Shader.hpp>
#include <cstdint>
#include <optional>
#include <vector>

class Board;

class BoardRenderer
{
public:

	BoardRenderer();

	// How the board is revealed once the game is over.
	enum class Reveal : std::uint8_t
	{
		None, // game still running, nothing is revealed
		Lost, // every mine is shown, wrong flags are marked
		Won   // unflagged mines are shown, flags are kept
	};

	struct State
	{
		Reveal reveal;
		std::optional<std::size_t> pressedCellIndex;
		const std::vector<std::size_t>& runningMineIndexes;
	};

	void resize(const Board& board);
	void update(const Board& board, const State& state);
	void render(sf::RenderTarget& target) const;

private:

	void setCellTexCoords(std::size_t index, const sf::FloatRect& atlasRect);

private:

	sf::VertexArray varray_;
	sf::Shader shader_;
};
