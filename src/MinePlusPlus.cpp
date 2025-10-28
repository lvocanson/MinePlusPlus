#include "App.h"
#include "Layer/MinesweeperLayer.h"
#include "Layer/GameUILayer.h"

int main()
{
	App app;
	app.pushLayer<MinesweeperLayer>();
	app.pushLayer<GameUILayer>();
	return app.run();
}
