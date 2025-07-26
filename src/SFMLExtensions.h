#include <SFML/Graphics.hpp>

namespace SFMLExtensions
{

void drawTexture(sf::RenderTarget& target, const sf::Texture& texture, const sf::FloatRect& rect);;
void setStringAndKeepOrigin(sf::Text& text, const sf::String& string);

} // namespace SFMLExtensions
