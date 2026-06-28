#include "ca.h"
#include <stdlib.h>
#include <string.h>

/* --- neighborhood index <-> cell vector (cells[0] = most significant) --- */
static void decode(long idx, int K, int N, int *cells) {
	for (int j = N - 1; j >= 0; j--) {
		cells[j] = (int)(idx % K);
		idx /= K;
	}
}
static long encode(const int *cells, int K, int N) {
	long idx = 0;
	for (int j = 0; j < N; j++)
		idx = idx * K + cells[j];
	return idx;
}
static int is_uniform(const int *cells, int N) {
	for (int j = 1; j < N; j++)
		if (cells[j] != cells[0])
			return 0;
	return 1;
}

/* Symmetry orbit of a neighborhood (deduplicated, encoded indices).
   1D: identity + reflection. 2D (N=5, layout [center,up,right,down,left]): 4
   rotations. */
static int variants_of(const Rule *r, long idx, long *out) {
	int cells[CA_MAXN];
	decode(idx, r->K, r->N, cells);
	int cnt = 0;
	if (r->dim == 1) {
		out[cnt++] = idx;
		int rev[CA_MAXN];
		for (int j = 0; j < r->N; j++)
			rev[j] = cells[r->N - 1 - j];
		long e = encode(rev, r->K, r->N);
		if (e != idx)
			out[cnt++] = e;
	} else {
		int cur[CA_MAXN];
		memcpy(cur, cells, sizeof(int) * r->N);
		for (int rot = 0; rot < 4; rot++) {
			long e = encode(cur, r->K, r->N);
			int dup = 0;
			for (int t = 0; t < cnt; t++)
				if (out[t] == e) {
					dup = 1;
					break;
				}
			if (!dup)
				out[cnt++] = e;
			/* rotate 90deg: [c,u,r,d,l] -> [c,l,u,r,d] */
			int u = cur[1], rr = cur[2], d = cur[3], l = cur[4];
			cur[1] = l;
			cur[2] = u;
			cur[3] = rr;
			cur[4] = d;
		}
	}
	return cnt;
}
static long orbit_min(const long *o, int n) {
	long m = o[0];
	for (int i = 1; i < n; i++)
		if (o[i] < m)
			m = o[i];
	return m;
}

/* strong quiescence: a neighborhood uniform in state s maps to s */
static void set_uniform(Rule *r) {
	int cells[CA_MAXN];
	for (int s = 0; s < r->K; s++) {
		for (int j = 0; j < r->N; j++)
			cells[j] = s;
		r->table[encode(cells, r->K, r->N)] = (uint8_t)s;
	}
}
static void fill_orbit(Rule *r, long idx, uint8_t v) {
	long orb[8];
	int n = variants_of(r, idx, orb);
	for (int i = 0; i < n; i++)
		r->table[orb[i]] = v;
}

Rule *ca_new(int K, int N, int dim) {
	if (K < 2 || K > CA_MAXK)
		return NULL;
	if (N < 1 || N > CA_MAXN)
		return NULL;
	if (dim == 1 && (N % 2) == 0)
		return NULL; /* need a center cell */
	if (dim == 2 && N != 5)
		return NULL; /* only the 5-cell template */
	if (dim != 1 && dim != 2)
		return NULL;

	Rule *r = calloc(1, sizeof(Rule));
	r->K = K;
	r->N = N;
	r->dim = dim;
	long sz = 1;
	for (int i = 0; i < N; i++)
		sz *= K;
	r->size = sz;
	r->table = calloc(sz, 1);
	r->canon = malloc(sizeof(long) * sz);
	r->canon_cnt = malloc(sizeof(int) * sz);
	r->ncanon = 0;

	int cells[CA_MAXN];
	long orb[8];
	for (long idx = 0; idx < sz; idx++) {
		decode(idx, K, N, cells);
		if (is_uniform(cells, N))
			continue;
		int n = variants_of(r, idx, orb);
		if (orbit_min(orb, n) != idx)
			continue; /* keep one rep per orbit */
		r->canon[r->ncanon] = idx;
		r->canon_cnt[r->ncanon] = n;
		r->ncanon++;
	}
	return r;
}

void ca_free(Rule *r) {
	if (!r)
		return;
	free(r->table);
	free(r->canon);
	free(r->canon_cnt);
	free(r);
}

void ca_gen_random(Rule *r, double lambda, Rng *rng) {
	memset(r->table, 0, r->size);
	set_uniform(r);
	for (long c = 0; c < r->ncanon; c++) {
		uint8_t v =
		    (rng_double(rng) < lambda)
			? (uint8_t)(1 +
				    rng_int(rng, r->K - 1)) /* non-quiescent */
			: 0;				    /* quiescent */
		fill_orbit(r, r->canon[c], v);
	}
}

void ca_walk_init(Rule *r) {
	memset(r->table, 0, r->size);
	set_uniform(r);
}

void ca_walk_to(Rule *r, double target, Rng *rng) {
	/* raise lambda by activating quiescent classes; lower by quiescing
	 * active ones */
	while (ca_realized_lambda(r) < target) {
		int done = 0;
		for (long t = 0; t < r->ncanon * 4 && !done; t++) {
			long c = rng_int(rng, r->ncanon);
			if (r->table[r->canon[c]] == 0) {
				fill_orbit(
				    r, r->canon[c],
				    (uint8_t)(1 + rng_int(rng, r->K - 1)));
				done = 1;
			}
		}
		if (!done)
			break;
	}
	while (ca_realized_lambda(r) > target) {
		int done = 0;
		for (long t = 0; t < r->ncanon * 4 && !done; t++) {
			long c = rng_int(rng, r->ncanon);
			if (r->table[r->canon[c]] != 0) {
				fill_orbit(r, r->canon[c], 0);
				done = 1;
			}
		}
		if (!done)
			break;
	}
}

double ca_realized_lambda(const Rule *r) {
	long n = 0;
	for (long i = 0; i < r->size; i++)
		if (r->table[i] != 0)
			n++;
	return (double)n / (double)r->size;
}

void ca_step_1d(const Rule *r, const uint8_t *cur, uint8_t *next, int size) {
	int rad = (r->N - 1) / 2;
	int K = r->K;
	for (int i = 0; i < size; i++) {
		long idx = 0;
		for (int j = -rad; j <= rad; j++) {
			int p = ((i + j) % size + size) % size;
			idx = idx * K + cur[p];
		}
		next[i] = r->table[idx];
	}
}

void ca_step_2d(const Rule *r, const uint8_t *cur, uint8_t *next, int w,
		int h) {
	int K = r->K;
	for (int y = 0; y < h; y++) {
		int yu = (y - 1 + h) % h, yd = (y + 1) % h;
		for (int x = 0; x < w; x++) {
			int xl = (x - 1 + w) % w, xr = (x + 1) % w;
			int c = cur[y * w + x];
			int u = cur[yu * w + x];
			int rr = cur[y * w + xr];
			int d = cur[yd * w + x];
			int l = cur[y * w + xl];
			long idx = ((((long)c * K + u) * K + rr) * K + d) * K +
				   l; /* [c,u,r,d,l] */
			next[y * w + x] = r->table[idx];
		}
	}
}
