#define B ((int)(sizeof(uint64) * 8))
#define BMASK (B - 1)
#define BITARRAY(name, N) uint64 name[((N) + B - 1) / B]
#define BIT(bitarray, i) (((bitarray)[(i) / B] >> ((i) & BMASK)) & 1ULL)
#define SETBIT(bitarray, i) ((bitarray)[(i) / B] |= (1ULL << ((i) & BMASK)))
#define CLEARBIT(bitarray, i) ((bitarray)[(i) / B] &= ~(1ULL << ((i) & BMASK)))
