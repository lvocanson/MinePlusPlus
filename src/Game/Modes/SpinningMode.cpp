#include "SpinningMode.h"
#include "Core/App.h"

namespace
{

constexpr float ROTATION_SPEED = 1.f;

}

SpinningMode::~SpinningMode()
{
	sf::View view = App::instance().window.getView();
	view.setRotation(sf::Angle{});
	App::instance().window.setView(view);
}

void SpinningMode::update(float dt)
{
	sf::View view = App::instance().window.getView();
	view.rotate(sf::degrees(ROTATION_SPEED * dt));
	App::instance().window.setView(view);
}
