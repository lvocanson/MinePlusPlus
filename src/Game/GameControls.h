#pragma once
#include "Core/AppEvent.h"

struct GameControls
{
	void operator()(const WorldEvent::Pressed& event);
	void operator()(const WorldEvent::Released& event);
};
