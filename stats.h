/* Information-theoretic measures over CA dynamics (Langton 1990, section 5). */
#ifndef STATS_H
#define STATS_H
#include "ca.h"

typedef struct {
	double H;	/* average single-cell Shannon entropy (bits)        */
	double H_norm;	/* H / log2(K), normalized to [0,1]                  */
	double mi_t;	/* mutual information: cell(t) vs cell(t+1)          */
	double mi_s;	/* mutual information: cell vs neighbor at distance 1 */
	long transient; /* steps until the configuration first recurs (cap)  */
} Measures;

/* Run a CA (1D: h=1, w=size; 2D: w=h=size) from a random init, settle, then
   sample `sample` steps to estimate the measures. Transient is measured on a
   fresh run, capped at tcap. */
Measures ca_measure(Rule *r, int w, int h, int settle, int sample, long tcap,
		    Rng *rng);

#endif
