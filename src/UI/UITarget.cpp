#include "UITarget.h"
#include "Game/Resources.h"
#include "Button.h"
#include "NumberField.h"
#include "Text.h"
#include "Cursor.h"

namespace
{

constexpr auto& FONT_REF = Resources::Fonts::roboto;
constexpr unsigned FONT_SIZE = 20;
constexpr int CURSOR_WIDTH = 2;
constexpr int CURSOR_DIST_FROM_TEXT = 2;

}

sf::RectangleShape UITarget::rect_{};
sf::Text UITarget::text_{FONT_REF};

UITarget::UITarget(sf::RenderTarget& target)
	: target_(target)
	, view_(target.getView())
{
	sf::Vector2f size(target.getSize());
	target.setView({size / 2.f, size});

	rect_.setOutlineThickness(2.f);
	rect_.setOutlineColor(sf::Color::White);

	text_.setCharacterSize(FONT_SIZE);
	text_.setFillColor(sf::Color::White);
	text_.setOutlineColor(sf::Color::Black);
	text_.setOutlineThickness(1.f);
}

UITarget::~UITarget()
{
	target_.setView(view_);
}

void UITarget::draw(const Button& button)
{
	sf::FloatRect rect(button.rect);
	rect_.setPosition(rect.position);
	rect_.setSize(rect.size);
	rect_.setFillColor({0x79, 0xB6, 0x1E, 0xFF});

	text_.setString(button.text.data());
	sf::FloatRect bounds = text_.getLocalBounds();
	text_.setOrigin(bounds.getCenter());
	text_.setPosition(rect.getCenter());

	target_.draw(rect_);
	target_.draw(text_);
}

void UITarget::draw(const Text& text)
{
	text_.setString(text.string.data());
	sf::FloatRect bounds = text_.getLocalBounds();
	switch (text.origin)
	{
	case Text::TopLeft: text_.setOrigin(bounds.position);
		break;
	case Text::Top: text_.setOrigin({bounds.position.x + bounds.size.x / 2.f, bounds.position.y});
		break;
	case Text::TopRight: text_.setOrigin(bounds.position + sf::Vector2f(bounds.size.x, 0.f));
		break;
	case Text::Left: text_.setOrigin({bounds.position.x, bounds.position.y + bounds.size.y / 2.f});
		break;
	case Text::Middle: text_.setOrigin(bounds.getCenter());
		break;
	case Text::Right: text_.setOrigin({bounds.position.x + bounds.size.x, bounds.position.y + bounds.size.y / 2.f});
		break;
	case Text::BotLeft: text_.setOrigin(bounds.position + sf::Vector2f(0.f, bounds.size.y));
		break;
	case Text::Bot: text_.setOrigin({bounds.position.x + bounds.size.x / 2.f, bounds.position.y + bounds.size.y});
		break;
	case Text::BotRight: text_.setOrigin(bounds.position + bounds.size);
		break;
	default: text_.setOrigin({0.f, 0.f});
		break;
	};
	text_.setPosition(sf::Vector2f(text.position));

	target_.draw(text_);
}

void UITarget::draw(const NumberField& field)
{
	sf::FloatRect rect(field.rect);
	rect_.setPosition(rect.position);
	rect_.setSize(rect.size);
	rect_.setFillColor(sf::Color::White);

	text_.setString(std::u32string{field.view()});
	sf::FloatRect bounds = text_.getLocalBounds();
	text_.setOrigin(bounds.getCenter());
	text_.setPosition(rect.getCenter());

	target_.draw(rect_);
	target_.draw(text_);
}

void UITarget::draw(const Cursor& cursor)
{
	if (!cursor.field)
		return;

	text_.setString(std::u32string{cursor.field->view()});
	sf::FloatRect bounds = text_.getLocalBounds();
	text_.setOrigin({bounds.position.x, bounds.position.y + bounds.size.y / 2.f});
	text_.setPosition(sf::FloatRect(cursor.field->rect).getCenter());

	sf::Vector2f cursorSize = {CURSOR_WIDTH, FONT_REF.getLineSpacing(FONT_SIZE) * 0.9f};
	rect_.setSize(cursorSize);
	rect_.setFillColor(sf::Color::Black);
	auto oldOutlineSz = rect_.getOutlineThickness();
	rect_.setOutlineThickness(0.f);
	rect_.setPosition(
		text_.getPosition() + sf::Vector2f(bounds.size.x / 2.f + CURSOR_DIST_FROM_TEXT, -cursorSize.y / 2.f));

	target_.draw(rect_);
	rect_.setOutlineThickness(oldOutlineSz);
}
