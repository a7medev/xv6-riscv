#include "types.h"

uint rand(uint *seed) {
    // Linear congruential psuedo-random number generator
    const uint a = 1664525;
    const uint c = 1013904223;
    const uint m = 0xFFFFFFFF; // 2^31

    *seed = (a * (*seed) + c) & m;
    return *seed;
}

uint randuntil(uint *seed, uint n) {
    return rand(seed) % n;
}
