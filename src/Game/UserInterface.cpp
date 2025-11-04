#include "UserInterface.h"
#include "Core/App.h"
#include "Utils/Overloaded.h"
#include "Game/Resources.h"
#include <format>

namespace
{

constexpr auto& FONT_REF = Resources::Fonts::roboto;
constexpr unsigned FONT_SIZE = 20;
constexpr float SCREEN_PADDING = 5.f;
constexpr float BUTTON_PADDING = 5.f;

}

UserInterface::UserInterface()
	: gameText_(FONT_REF)
	, restartText_(FONT_REF)
	, mainMenuText_(FONT_REF)
	, resetCameraText_(FONT_REF)
{
	setupText(gameText_);
	restartText_.setString("Restart");
	sf::Vector2f restartSize = setupText(restartText_);
	mainMenuText_.setString("Main Menu");
	sf::Vector2f mainMenuSize = setupText(mainMenuText_);
	resetCameraText_.setString("Reset Camera");
	sf::Vector2f resetCameraSize = setupText(resetCameraText_);

	sf::Vector2f btnSize
	{
		std::max({restartSize.x, mainMenuSize.x, resetCameraSize.x}) + BUTTON_PADDING,
		std::max({restartSize.y, mainMenuSize.y, resetCameraSize.y}) + BUTTON_PADDING,
	};

	setupBtn(restartBtn_, btnSize);
	setupBtn(mainMenuBtn_, btnSize);
	setupBtn(resetCameraBtn_, btnSize);

	App::instance().clearColor = {0x79, 0x31, 0x32, 0x00};
	App::instance().game.setMedium();
	resizeUI(sf::Vector2f(App::instance().window.getSize()));
	centerBoardOnView();
}

sf::Vector2f UserInterface::setupText(sf::Text& text) const
{
	text.setCharacterSize(FONT_SIZE);
	text.setFillColor(sf::Color::White);
	text.setOutlineColor(sf::Color::Black);
	text.setOutlineThickness(1.f);
	sf::FloatRect bounds = text.getLocalBounds();
	text.setOrigin(bounds.position + bounds.size / 2.f);
	return bounds.size;
}

void UserInterface::setupBtn(sf::RectangleShape& rect, sf::Vector2f size) const
{
	rect.setSize(size);
	rect.setFillColor({0x79, 0xB6, 0x1E, 0xFF});
	rect.setOutlineThickness(BUTTON_PADDING / 3.f);
	rect.setOutlineColor(sf::Color::White);
}

EventConsumed UserInterface::handleEvent(const sf::Event& event)
{
	Overloaded visitor
	{
		[&](const sf::Event::Resized& resized)
		{
			resizeUI(sf::Vector2f(resized.size));
			return EventConsumed::No;
		},

		[&](const sf::Event::MouseButtonPressed& mbPressed)
		{
			auto position = sf::Vector2f(mbPressed.position);
			if (restartBtn_.getGlobalBounds().contains(position))
			{
				App::instance().game.restart();
			}
			else if (mainMenuBtn_.getGlobalBounds().contains(position))
			{
				// TODO
			}
			else if (resetCameraBtn_.getGlobalBounds().contains(position))
			{
				centerBoardOnView();
			}
			return EventConsumed::No;
		},

		[](const auto&)
		{
			return EventConsumed::No;
		}
	};

	return event.visit(visitor);
}

void UserInterface::resizeUI(sf::Vector2f newSize)
{
	uiView_.setSize(newSize);
	uiView_.setCenter(newSize / 2.f);

	gameText_.setPosition({SCREEN_PADDING, SCREEN_PADDING});

	sf::Vector2f restartSize = restartBtn_.getSize();
	sf::Vector2f restartPosition
	{
		newSize.x - SCREEN_PADDING - restartSize.x,
		SCREEN_PADDING
	};
	restartBtn_.setPosition(restartPosition);
	restartText_.setPosition(restartPosition + restartSize / 2.f);

	sf::Vector2f mainMenuSize = mainMenuBtn_.getSize();
	sf::Vector2f mainMenuPos
	{
		SCREEN_PADDING,
		newSize.y - SCREEN_PADDING - mainMenuSize.y
	};
	mainMenuBtn_.setPosition(mainMenuPos);
	mainMenuText_.setPosition(mainMenuPos + mainMenuSize / 2.f);

	sf::Vector2f resetCameraSize = resetCameraBtn_.getSize();
	sf::Vector2f resetCameraPos
	{
		newSize.x - SCREEN_PADDING - resetCameraSize.x,
		newSize.y - SCREEN_PADDING - resetCameraSize.y
	};
	resetCameraBtn_.setPosition(resetCameraPos);
	resetCameraText_.setPosition(resetCameraPos + resetCameraSize / 2.f);
}

void UserInterface::centerBoardOnView()
{
	auto& board = App::instance().game.getBoard();
	sf::Vector2f size = {float(board.getSize().x), float(board.getSize().y)};
	sf::Vector2f center = {float(size.x) * .5f, float(size.y) * .5f};

	auto windowSize = sf::Vector2f(App::instance().window.getSize());
	float aspect = windowSize.x / windowSize.y;
	float verticalSize = std::max(size.x, size.y * aspect);

	sf::View view = App::instance().window.getView();
	view.setCenter(center);
	view.setSize({verticalSize * aspect, verticalSize});
	App::instance().window.setView(view);
}

void UserInterface::update(float dt)
{
	auto& board = App::instance().game.getBoard();
	std::make_signed_t<std::size_t> minesLeft = board.getMineCount() - board.getFlagCount();
	float bestTime = std::numeric_limits<float>::signaling_NaN(); // TODO
	auto timeInSeconds = App::instance().game.getPlayingTime().asMicroseconds() / 1'000'000;

	auto str = std::format("Mines left: {}\nBest time: {}\nTime: {}s",
		minesLeft, bestTime, timeInSeconds);
	gameText_.setString(str);
}

void UserInterface::render(sf::RenderTarget& target) const
{
	sf::View gameView = target.getView();
	target.setView(uiView_);
	target.draw(resetCameraBtn_);
	target.draw(mainMenuBtn_);
	target.draw(restartBtn_);
	target.draw(resetCameraText_);
	target.draw(mainMenuText_);
	target.draw(restartText_);
	target.draw(gameText_);
	target.setView(gameView);
}
