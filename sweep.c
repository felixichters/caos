/* Lambda sweep: many random tables per lambda, emit measures as TSV (Langton
 * 1990, section 5). */
#include "args.h"
#include "ca.h"
#include "stats.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

void run_sweep(int argc, char **argv) {
	int dim = atoi(arg_str(argc, argv, "--dim", "2"));
	int K = atoi(arg_str(argc, argv, "--k", dim == 1 ? "4" : "8"));
	int N = atoi(arg_str(argc, argv, "--n", "5"));
	int size = atoi(arg_str(argc, argv, "--size", dim == 1 ? "128" : "64"));
	double lmin = atof(arg_str(argc, argv, "--lmin", "0.0"));
	double lmax = atof(arg_str(argc, argv, "--lmax", "-1"));
	double lstep = atof(arg_str(argc, argv, "--lstep", "0.01"));
	int runs = atoi(arg_str(argc, argv, "--runs", "10"));
	int settle = atoi(arg_str(argc, argv, "--settle", "100"));
	int sample = atoi(arg_str(argc, argv, "--sample", "100"));
	long tcap = atol(arg_str(argc, argv, "--tcap", "2000"));
	int agg = arg_flag(argc, argv, "--aggregate");
	uint64_t seed = strtoull(arg_str(argc, argv, "--seed", "1"), NULL, 10);

	if (lmax < 0)
		lmax = 1.0 - 1.0 / K;

	Rule *r = ca_new(K, N, dim);
	if (!r) {
		fprintf(stderr, "sweep: invalid params (need 2<=K<=8; dim=1 "
				"odd N; dim=2 N=5)\n");
		exit(1);
	}
	Rng rng;
	rng_seed(&rng, seed);
	int w = (dim == 1) ? size : size;
	int h = (dim == 1) ? 1 : size;

	printf(
	    "# caos sweep dim=%d K=%d N=%d size=%d runs=%d settle=%d sample=%d "
	    "tcap=%ld\n",
	    dim, K, N, size, runs, settle, sample, tcap);
	printf("# lambda\tH\tH_norm\tmi_temporal\tmi_spatial\ttransient\n");

	for (double L = lmin; L <= lmax + 1e-9; L += lstep) {
		double sH = 0, sHn = 0, smt = 0, sms = 0, stt = 0;
		int cnt = 0;
		for (int run = 0; run < runs; run++) {
			ca_gen_random(r, L, &rng);
			double rl = ca_realized_lambda(r);
			Measures m =
			    ca_measure(r, w, h, settle, sample, tcap, &rng);
			if (agg) {
				sH += m.H;
				sHn += m.H_norm;
				smt += m.mi_t;
				sms += m.mi_s;
				stt += m.transient;
				cnt++;
			} else {
				printf("%.4f\t%.5f\t%.5f\t%.5f\t%.5f\t%ld\n",
				       rl, m.H, m.H_norm, m.mi_t, m.mi_s,
				       m.transient);
			}
		}
		if (agg && cnt) {
			printf("%.4f\t%.5f\t%.5f\t%.5f\t%.5f\t%.2f\n", L,
			       sH / cnt, sHn / cnt, smt / cnt, sms / cnt,
			       stt / cnt);
		}
		fflush(stdout);
	}
	ca_free(r);
}
