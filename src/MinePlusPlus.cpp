#include "App.h"
#include "Layer/WindowInput.h"
#include "Game/BoardRenderer.h"
#include "Game/MinesweeperInput.h"
#include "Layer/GameUILayer.h"

int main()
{
	App app;
	app.layerStack.scheduleAsyncCommand<LayerStack::Push>(std::make_unique<WindowInput>());
	app.layerStack.scheduleAsyncCommand<LayerStack::Push>(std::make_unique<BoardRenderer>());
	app.layerStack.scheduleAsyncCommand<LayerStack::Push>(std::make_unique<MinesweeperInput>());
	app.layerStack.scheduleAsyncCommand<LayerStack::Push>(std::make_unique<GameUILayer>());
	return app.run();
}
