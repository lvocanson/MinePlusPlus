#pragma once

struct mfb_window;

namespace Profiling
{

void update(mfb_window* mainWindow, float dt);
void shutdown();

} // namespace Profiling
