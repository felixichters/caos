/* 1D space-time visualization (Langton 1990, section 3). */
#include "args.h"
#include "ca.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *GLYPHS = " .:-=+*#"; /* CA_MAXK glyphs, state 0 = blank */

static void show_one(Rule *r, int size, int steps, const char *init, Rng *rng) {
	uint8_t *cur = calloc(size, 1), *next = calloc(size, 1), *tmp;
	if (strcmp(init, "patch") == 0) {
		int start = size / 2 - 10;
		for (int i = 0; i < 20; i++)
			cur[((start + i) % size + size) % size] =
			    (uint8_t)rng_int(rng, r->K);
	} else {
		for (int i = 0; i < size; i++)
			cur[i] = (uint8_t)rng_int(rng, r->K);
	}
	for (int t = 0; t < steps; t++) {
		for (int i = 0; i < size; i++)
			putchar(GLYPHS[cur[i]]);
		putchar('\n');
		ca_step_1d(r, cur, next, size);
		tmp = cur;
		cur = next;
		next = tmp;
	}
	free(cur);
	free(next);
}

void run_viz(int argc, char **argv) {
	int K = atoi(arg_str(argc, argv, "--k", "4"));
	int N = atoi(arg_str(argc, argv, "--n", "5"));
	int size = atoi(arg_str(argc, argv, "--size", "128"));
	int steps = atoi(arg_str(argc, argv, "--steps", "100"));
	uint64_t seed = strtoull(arg_str(argc, argv, "--seed", "1"), NULL, 10);
	const char *init = arg_str(argc, argv, "--init", "random");
	double lambda = atof(arg_str(argc, argv, "--lambda", "0.45"));
	int sweep = arg_flag(argc, argv, "--sweep");

	Rule *r = ca_new(K, N, 1);
	if (!r) {
		fprintf(stderr,
			"viz: invalid params (need 2<=K<=8, odd N, dim=1)\n");
		exit(1);
	}
	Rng rng;
	rng_seed(&rng, seed);

	if (sweep) {
		double lmax = 1.0 - 1.0 / K;
		ca_walk_init(r);
		for (double L = 0.05; L <= lmax + 1e-9; L += 0.05) {
			ca_walk_to(r, L, &rng);
			printf("\n=== lambda ~ %.2f (realized %.3f) ===\n", L,
			       ca_realized_lambda(r));
			show_one(r, size, steps, init, &rng);
		}
	} else {
		ca_gen_random(r, lambda, &rng);
		printf("# K=%d N=%d size=%d  lambda target %.3f realized %.3f  "
		       "init=%s\n",
		       K, N, size, lambda, ca_realized_lambda(r), init);
		show_one(r, size, steps, init, &rng);
	}
	ca_free(r);
}
