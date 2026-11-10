#ifdef _WIN32
#define _CRT_RAND_S
#include <stdlib.h>
#else
#include <sys/random.h>
#endif

#include "SeededRandom.h"

std::uint32_t makeSystemSeed()
{
#ifdef _WIN32
	unsigned int value;
	return rand_s(&value) == 0 ? std::uint32_t(value) : 0;
#else
	std::uint32_t value;
	return getrandom(&value, sizeof(value), 0) == ssize_t(sizeof(value)) ? value : 0;
#endif
}
