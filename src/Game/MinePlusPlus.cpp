#include "Core/App.h"
#include "Core/WindowInput.h"
#include "Game/BoardRenderer.h"
#include "Game/MinesweeperInput.h"
#include "Game/UserInterface.h"

int main()
{
	App app;
	app.layerStack.scheduleAsyncCommand<LayerStack::Push>(std::make_unique<WindowInput>());
	app.layerStack.scheduleAsyncCommand<LayerStack::Push>(std::make_unique<BoardRenderer>());
	app.layerStack.scheduleAsyncCommand<LayerStack::Push>(std::make_unique<MinesweeperInput>());
	app.layerStack.scheduleAsyncCommand<LayerStack::Push>(std::make_unique<UserInterface>());
	return app.run();
}
