#pragma once
#include <cstddef>

namespace MinesweeperEvents
{

struct SetSelectedCell { std::size_t cell; };

enum class RenderMode { Off = 0, Classic, RevealAll };
struct SetRenderMode { RenderMode mode; };

}
