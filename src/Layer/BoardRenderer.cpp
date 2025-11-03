#include "BoardRenderer.h"
#include "App.h"
#include "Resources.h"
#include "Utils/SFMLExtensions.h"

using namespace MinesweeperEvents;

BoardRenderer::BoardRenderer()
	: selectedCell_(-1)
	, renderMode_()
{
	auto handle = EventBus::SubscriptionHandle(this);
	App::instance().eventBus.subscribe<SetSelectedCell>([this](const SetSelectedCell& event) { selectedCell_ = event.cell; }, handle);
	App::instance().eventBus.subscribe<SetRenderMode>([this](const SetRenderMode& event) { renderMode_ = event.mode; }, handle);
}

BoardRenderer::~BoardRenderer()
{
	auto handle = EventBus::SubscriptionHandle(this);
	App::instance().eventBus.unsubscribe<SetSelectedCell>(handle);
	App::instance().eventBus.unsubscribe<SetRenderMode>(handle);
}

void BoardRenderer::render(sf::RenderTarget& target) const
{
	if (renderMode_ == RenderMode::Off)
		return;

	auto& board = App::instance().board;
	for (std::size_t index = 0; index < board.getCells().size(); ++index)
	{
		const sf::Texture* texture = nullptr;
		auto& cell = board.getCellAt(index);

		if (cell.flagged)
		{
			texture = (renderMode_ == RenderMode::RevealAll && !cell.mined)
				? &Resources::Textures::openedCellNoMine
				: &Resources::Textures::unopenedFlaggedCell;
		}

		else if (!cell.opened)
		{
			texture = (renderMode_ == RenderMode::RevealAll && cell.mined)
				? &Resources::Textures::openedCellMine
				: (selectedCell_ == index)
				? &Resources::Textures::unopenedSelectedCell
				: &Resources::Textures::unopenedCell;
		}

		else if (cell.mined)
		{
			texture = &Resources::Textures::openedCellClickedMine;
		}

		else
		{
			constexpr const sf::Texture* openedCellTextures[]
			{
				&Resources::Textures::openedCell0,
				&Resources::Textures::openedCell1,
				&Resources::Textures::openedCell2,
				&Resources::Textures::openedCell3,
				&Resources::Textures::openedCell4,
				&Resources::Textures::openedCell5,
				&Resources::Textures::openedCell6,
				&Resources::Textures::openedCell7,
				&Resources::Textures::openedCell8,
			};

			assert(cell.adjacentMines < 9);
			texture = openedCellTextures[cell.adjacentMines];
		}

		assert(texture);

		auto [x, y] = board.toCoordinates(index);
		const sf::FloatRect rect{{float(x), float(y)}, {1.f, 1.f}};
		SFMLExtensions::drawTexture(target, *texture, rect);
	}
}
