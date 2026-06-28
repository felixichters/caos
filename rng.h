/* Tiny, fast PRNG (xorshift64*) — no libc rand() global state. */
#ifndef RNG_H
#define RNG_H
#include <stdint.h>

typedef struct {
	uint64_t s;
} Rng;

static inline void rng_seed(Rng *r, uint64_t seed) {
	r->s = seed ? seed : 0x9E3779B97F4A7C15ULL;
}
static inline uint64_t rng_next(Rng *r) {
	uint64_t x = r->s;
	x ^= x >> 12;
	x ^= x << 25;
	x ^= x >> 27;
	r->s = x;
	return x * 0x2545F4914F6CDD1DULL;
}
/* uniform in [0,1) */
static inline double rng_double(Rng *r) {
	return (rng_next(r) >> 11) * (1.0 / 9007199254740992.0);
}
/* uniform in [0,n) */
static inline uint32_t rng_int(Rng *r, uint32_t n) {
	return (uint32_t)(rng_double(r) * n);
}
#endif
