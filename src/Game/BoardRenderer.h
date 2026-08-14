#pragma once
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/Graphics/Shader.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <array>
#include <cstdint>

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
	void makeDirty() { dirty_ = true; }

private:

	void flushScratch(std::size_t first, std::size_t count);

private:

	sf::VertexArray boardQuad_;
	sf::Texture stateTexture_;
	std::array<std::uint8_t, 512> scratch_;

	sf::Shader shader_;
	bool dirty_;
};
