#include <random>

// 64 bits number generator
inline thread_local std::mt19937_64 gen(std::random_device{}());
