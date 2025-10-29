#include "App.h"
#include "Layer/BoardRenderer.h"
#include "Layer/GameUILayer.h"

int main()
{
	App app;
	app.pushLayer<BoardRenderer>();
	app.pushLayer<GameUILayer>();
	return app.run();
}
