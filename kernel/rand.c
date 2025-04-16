#include "rand.h"

int rand(int *seed) {
    // Linear congruential psuedo-random number generator
    const int a = 1664525;
    const int c = 1013904223;
    const int m = 0x7FFFFFFF; // 2^31

    *seed = (a * (*seed) + c) & m;
    return *seed;
}

int randuntil(int *seed, int n) {
    return rand(seed) % n;
}
