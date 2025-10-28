#include "MinesweeperLayer.h"
#include "Resources.h"
#include "Event/EventBus.h"
#include "SFMLExtensions.h"

using namespace MinesweeperEvents;

MinesweeperLayer::MinesweeperLayer()
	: selectedCell_(-1)
	, renderMode_()
{
	auto handle = EventBus::SubscriptionHandle(this);
	EventBus::subscribe<BoardRequest>([this](const BoardRequest& event) { event.board = &board_; }, handle);
	EventBus::subscribe<SetSelectedCell>([this](const SetSelectedCell& event) { selectedCell_ = event.cell; }, handle);
	EventBus::subscribe<SetRenderMode>([this](const SetRenderMode& event) { renderMode_ = event.mode; }, handle);
}

MinesweeperLayer::~MinesweeperLayer()
{
	auto handle = EventBus::SubscriptionHandle(this);
	EventBus::unsubscribe<BoardRequest>(handle);
	EventBus::unsubscribe<SetSelectedCell>(handle);
	EventBus::unsubscribe<SetRenderMode>(handle);
}

void MinesweeperLayer::render(sf::RenderTarget& target) const
{
	if (renderMode_ == RenderMode::Off)
		return;

	for (std::size_t index = 0; index < board_.getCells().size(); ++index)
	{
		const sf::Texture* texture = nullptr;
		auto& cell = board_.getCellAt(index);

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

		auto [x, y] = board_.toCoordinates(index);
		const sf::FloatRect rect{{float(x), float(y)}, {1.f, 1.f}};
		SFMLExtensions::drawTexture(target, *texture, rect);
	}
}
