#include "SFMLExtensions.h"

namespace SFMLExtensions
{

void drawTexture(sf::RenderTarget& target, const sf::Texture& texture, const sf::FloatRect& rect)
{
	sf::Vector2f texSize{texture.getSize()};
	sf::Vertex quad[4] =
	{
		{
			.position = rect.position,
			.texCoords = {0.f, 0.f}
		},
		{
			.position = rect.position + sf::Vector2f(rect.size.x, 0.f),
			.texCoords = {texSize.x, 0.f}
		},
		{
			.position = rect.position + rect.size,
			.texCoords = texSize
		},
		{
			.position = rect.position + sf::Vector2f(0, rect.size.y),
			.texCoords = {0, texSize.y}
		}
	};

	auto states = sf::RenderStates::Default;
	states.texture = &texture;

	target.draw(quad, 4, sf::PrimitiveType::TriangleFan, states);
}

void setStringAndKeepOrigin(sf::Text& text, const sf::String& string)
{
	auto oldSize = text.getLocalBounds().size;
	auto origin = text.getOrigin();
	text.setString(string);

	auto newSize = text.getLocalBounds().size;
	if (oldSize.x) origin *= newSize.x / oldSize.x;
	if (oldSize.y) origin *= newSize.y / oldSize.y;
	text.setOrigin(origin);
}

}
