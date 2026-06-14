/* Cellular automaton engine: lambda-parameterized transition tables (Langton 1990). */
#ifndef CA_H
#define CA_H
#include <stdint.h>
#include "rng.h"

#define CA_MAXN 9   /* max neighborhood size supported */
#define CA_MAXK 8   /* max states (keeps histograms small, table in uint8) */

typedef struct {
    int  K, N, dim;     /* states, neighborhood size, dimension (1 or 2) */
    long size;          /* K^N, number of transition-table entries */
    uint8_t *table;     /* table[neighborhood index] -> next state */
    long *canon;        /* non-uniform canonical (symmetry) representatives */
    int  *canon_cnt;    /* orbit size (table cells) per representative */
    long  ncanon;
} Rule;

Rule  *ca_new(int K, int N, int dim);   /* NULL on invalid params */
void   ca_free(Rule *r);

/* random-table method: each canonical class -> Sq with prob (1-lambda), else random non-Sq */
void   ca_gen_random(Rule *r, double lambda, Rng *rng);

/* table-walk-through method: start quiescent (lambda~0), perturb the same table toward target */
void   ca_walk_init(Rule *r);
void   ca_walk_to(Rule *r, double target, Rng *rng);

double ca_realized_lambda(const Rule *r);

void   ca_step_1d(const Rule *r, const uint8_t *cur, uint8_t *next, int size);
void   ca_step_2d(const Rule *r, const uint8_t *cur, uint8_t *next, int w, int h);

#endif
