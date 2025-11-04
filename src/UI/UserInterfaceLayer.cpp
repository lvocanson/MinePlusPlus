#include "UserInterfaceLayer.h"
#include "Game/Resources.h"
#include "Button.h"
#include "Text.h"

namespace
{

constexpr auto& FONT_REF = Resources::Fonts::roboto;
constexpr unsigned FONT_SIZE = 20;

}

EventConsumed UserInterfaceLayer::handleEvent(const sf::Event& event)
{
	if (auto* resized = event.getIf<sf::Event::Resized>())
	{
		sf::Vector2f size(resized->size);
		uiView_.setSize(size);
		uiView_.setCenter(size / 2.f);
		onScreenResized(sf::Vector2i(resized->size));
	}
	return EventConsumed::No;
}

void UserInterfaceLayer::render(sf::RenderTarget& target) const
{
	sf::View targetView = target.getView();
	target.setView(uiView_);
	{
		UITarget uiTarget(target);
		render(uiTarget);
	}
	target.setView(targetView);
}

UserInterfaceLayer::UITarget::UITarget(sf::RenderTarget& target)
	: target_(target)
	, text_(FONT_REF)
{
	rect_.setFillColor({0x79, 0xB6, 0x1E, 0xFF});
	rect_.setOutlineThickness(2.f);
	rect_.setOutlineColor(sf::Color::White);

	text_.setCharacterSize(FONT_SIZE);
	text_.setFillColor(sf::Color::White);
	text_.setOutlineColor(sf::Color::Black);
	text_.setOutlineThickness(1.f);
}

void UserInterfaceLayer::UITarget::draw(const Button& button)
{
	sf::FloatRect btnRect(button.rect);
	rect_.setPosition(btnRect.position);
	rect_.setSize(btnRect.size);

	// sf::String can't be constructed from a view
	sf::String str = sf::String::fromUtf8(button.text.begin(), button.text.end());
	text_.setString(str);
	sf::FloatRect bounds = text_.getLocalBounds();
	text_.setOrigin(bounds.getCenter());
	text_.setPosition(btnRect.getCenter());

	target_.draw(rect_);
	target_.draw(text_);
}

void UserInterfaceLayer::UITarget::draw(const Text& text)
{
	text_.setString(text.string);
	sf::FloatRect bounds = text_.getLocalBounds();
	switch (text.origin)
	{
	case Text::TopLeft: text_.setOrigin(bounds.position); break;
	case Text::Top: text_.setOrigin({bounds.position.x + bounds.size.x / 2.f, bounds.position.y}); break;
	case Text::TopRight: text_.setOrigin(bounds.position + sf::Vector2f(bounds.size.x, 0.f)); break;
	case Text::Left: text_.setOrigin({bounds.position.x, bounds.position.y + bounds.size.y / 2.f}); break;
	case Text::Middle: text_.setOrigin(bounds.getCenter()); break;
	case Text::Right: text_.setOrigin({bounds.position.x + bounds.size.x, bounds.position.y + bounds.size.y / 2.f}); break;
	case Text::BotLeft: text_.setOrigin(bounds.position + sf::Vector2f(0.f, bounds.size.y)); break;
	case Text::Bot: text_.setOrigin({bounds.position.x + bounds.size.x / 2.f, bounds.position.y + bounds.size.y}); break;
	case Text::BotRight: text_.setOrigin(bounds.position + bounds.size); break;
	default: text_.setOrigin({0.f, 0.f}); break;
	};
	text_.setPosition(sf::Vector2f(text.position));

	target_.draw(text_);
}
