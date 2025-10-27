#include "App.h"
#include "Layer/MinesweeperLayer.h"

int main()
{
	App app;
	app.pushLayer<MinesweeperLayer>(Vec2s{20, 20}, 21);
	return app.run();
}
