#include "stats.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

static void step_grid(const Rule *r, const uint8_t *cur, uint8_t *next, int w,
		      int h) {
	if (r->dim == 1)
		ca_step_1d(r, cur, next, w);
	else
		ca_step_2d(r, cur, next, w, h);
}

static double entropy_hist(const long *c, int K, long total) {
	if (!total)
		return 0.0;
	double H = 0.0;
	for (int i = 0; i < K; i++) {
		if (c[i]) {
			double p = (double)c[i] / total;
			H -= p * log2(p);
		}
	}
	return H;
}

/* I(A;B) = sum p(a,b) log2( p(a,b) / (p(a)p(b)) ) from a K*K joint histogram.
 */
static double mi_from_joint(const long *j, int K, long total) {
	if (!total)
		return 0.0;
	double pa[CA_MAXK] = {0}, pb[CA_MAXK] = {0};
	for (int a = 0; a < K; a++)
		for (int b = 0; b < K; b++) {
			double p = (double)j[a * K + b] / total;
			pa[a] += p;
			pb[b] += p;
		}
	double I = 0.0;
	for (int a = 0; a < K; a++)
		for (int b = 0; b < K; b++) {
			long c = j[a * K + b];
			if (!c)
				continue;
			double p = (double)c / total;
			I += p * log2(p / (pa[a] * pb[b]));
		}
	return I;
}

static uint64_t fnv(const uint8_t *d, int n) {
	uint64_t h = 1469598103934665603ULL;
	for (int i = 0; i < n; i++) {
		h ^= d[i];
		h *= 1099511628211ULL;
	}
	return h ? h : 1; /* reserve 0 as "empty" slot marker */
}

static long transient_len(Rule *r, int w, int h, long tcap, Rng *rng) {
	int n = w * h;
	uint8_t *cur = malloc(n), *next = malloc(n), *tmp;
	for (int i = 0; i < n; i++)
		cur[i] = (uint8_t)rng_int(rng, r->K);

	size_t cap = 1;
	while (cap < (size_t)(tcap * 2 + 8))
		cap <<= 1;
	uint64_t *set = calloc(cap, sizeof(uint64_t));
	size_t mask = cap - 1;
	long result = tcap;

	for (long t = 0; t < tcap; t++) {
		uint64_t hsh = fnv(cur, n);
		size_t p = hsh & mask;
		int found = 0;
		while (set[p]) {
			if (set[p] == hsh) {
				found = 1;
				break;
			}
			p = (p + 1) & mask;
		}
		if (found) {
			result = t;
			break;
		}
		set[p] = hsh;
		step_grid(r, cur, next, w, h);
		tmp = cur;
		cur = next;
		next = tmp;
	}
	free(set);
	free(cur);
	free(next);
	return result;
}

Measures ca_measure(Rule *r, int w, int h, int settle, int sample, long tcap,
		    Rng *rng) {
	int n = w * h, K = r->K;
	uint8_t *cur = malloc(n), *next = malloc(n), *tmp;
	for (int i = 0; i < n; i++)
		cur[i] = (uint8_t)rng_int(rng, r->K);

	for (int t = 0; t < settle; t++) {
		step_grid(r, cur, next, w, h);
		tmp = cur;
		cur = next;
		next = tmp;
	}

	long single[CA_MAXK] = {0};
	long jt[CA_MAXK * CA_MAXK] = {0}; /* temporal joint: cur vs next */
	long js[CA_MAXK * CA_MAXK] = {
	    0}; /* spatial joint: cur vs right neighbor */
	long total = 0, totalt = 0, totals = 0;

	for (int t = 0; t < sample; t++) {
		step_grid(r, cur, next, w, h);
		for (int i = 0; i < n; i++) {
			single[cur[i]]++;
			total++;
			jt[cur[i] * K + next[i]]++;
			totalt++;
		}
		if (r->dim == 1) {
			for (int i = 0; i < n; i++) {
				int jx = (i + 1) % w;
				js[cur[i] * K + cur[jx]]++;
				totals++;
			}
		} else {
			for (int y = 0; y < h; y++)
				for (int x = 0; x < w; x++) {
					int i = y * w + x,
					    jx = y * w + (x + 1) % w;
					js[cur[i] * K + cur[jx]]++;
					totals++;
				}
		}
		tmp = cur;
		cur = next;
		next = tmp;
	}

	Measures m;
	m.H = entropy_hist(single, K, total);
	m.H_norm = m.H / log2((double)K);
	m.mi_t = mi_from_joint(jt, K, totalt);
	m.mi_s = mi_from_joint(js, K, totals);
	free(cur);
	free(next);

	m.transient = transient_len(r, w, h, tcap, rng);
	return m;
}
