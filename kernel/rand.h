#include "types.h"

// Generates a random number x, 0 <= x < 2^32
uint rand(uint *seed);

// Generates a random number x, 0 <= x < n
uint randuntil(uint *seed, uint n);
