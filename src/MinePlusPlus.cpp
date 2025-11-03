#include "App.h"
#include "Layer/BoardRenderer.h"
#include "Layer/GameUILayer.h"

int main()
{
	App app;
	app.layerStack.scheduleAsyncCommand<LayerStack::Push>(std::make_unique<BoardRenderer>());
	app.layerStack.scheduleAsyncCommand<LayerStack::Push>(std::make_unique<GameUILayer>());
	return app.run();
}
