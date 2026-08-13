#pragma once
#include <SFML/Window/Event.hpp>
#include <variant>

struct AppEvent
{
	class App& app;
};

struct UIEvent
{
	template <class Event>
	UIEvent(Event&& data) : data_(data) {}

	struct Resized : AppEvent
	{
		sf::Vector2i newSize;
	};

	struct Pressed : AppEvent
	{
		sf::Mouse::Button button;
		sf::Vector2i position;
	};

	struct Released : AppEvent
	{
		sf::Mouse::Button button;
		sf::Vector2i position;
	};

	struct Typed : AppEvent
	{
		char32_t utf32;
	};

	enum Result
	{
		Ignored = 0, // Event will be passed down
		Consumed,    // Event will not be passed down
	};

	Result visit(auto& visitor) const
	{
		return std::visit(visitor, data_);
	}

private:

	std::variant<Resized, Pressed, Released, Typed> data_;
};

struct WorldEvent
{
	template <class Event>
	WorldEvent(Event&& data) : data_(data) {}

	struct Pressed : AppEvent
	{
		sf::Mouse::Button button;
		sf::Vector2f position;
	};

	struct Released : AppEvent
	{
		sf::Mouse::Button button;
		sf::Vector2f position;
	};

	void visit(auto& visitor) const
	{
		return std::visit(visitor, data_);
	}

private:

	std::variant<Pressed, Released> data_;
};
