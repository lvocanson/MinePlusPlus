#pragma once
#include "Text.h"
#include "Core/Canvas.h"

/*
 * Draws the UI in screen space.
 * Text rendering does not allocate.
 */
class UITarget
{
	friend class App;
	explicit UITarget(Canvas canvas) : canvas_(canvas) {}

public:

	void draw(const struct Button& button);
	void draw(const Text& text);
	void draw(const struct NumberField& field);
	void draw(const struct Cursor& cursor);

private:

	template <class Sequence>
	void pushText(Sequence&& sequence, Vec2f position, Text::Origin origin, Color color);

	Canvas canvas_;
};
