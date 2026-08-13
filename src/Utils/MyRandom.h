#pragma once
#include <random>

// 64 bits number generator
inline std::mt19937_64 gen(std::random_device{}());
