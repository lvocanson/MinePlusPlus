#include "Core/App.h"
#include "Core/WindowInput.h"
#include "Menus/MainMenu.h"

int main()
{
	App app;
	app.layerStack.scheduleAsyncCommand<LayerStack::Push>(std::make_unique<WindowInput>());
	app.layerStack.scheduleAsyncCommand<LayerStack::Push>(std::make_unique<MainMenu>());
	return app.run();
}
