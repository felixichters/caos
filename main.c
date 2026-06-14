#include <stdio.h>
#include <string.h>

void run_viz(int argc, char **argv);
void run_sweep(int argc, char **argv);

static void usage(const char *prog) {
    fprintf(stderr,
        "caos - experiments from Langton's \"Computation at the Edge of Chaos\"\n\n"
        "usage:\n"
        "  %s viz   [--lambda L | --sweep] [--size 128] [--steps 100]\n"
        "           [--k 4 --n 5] [--init random|patch] [--seed S]\n"
        "  %s sweep [--dim 1|2] [--k 8 --n 5] [--size 64]\n"
        "           [--lmin 0 --lmax <1-1/K> --lstep 0.01] [--runs 10]\n"
        "           [--settle 100 --sample 100 --tcap 2000] [--aggregate] [--seed S]\n\n"
        "  viz   -> 1D ASCII space-time diagram (defaults: K=4 N=5, section 3)\n"
        "  sweep -> TSV of entropy/mutual-info/transient per run (defaults: 2D K=8 N=5, section 5)\n",
        prog, prog);
}

int main(int argc, char **argv) {
    if (argc < 2) { usage(argv[0]); return 1; }
    if      (strcmp(argv[1], "viz")   == 0) run_viz(argc - 1, argv + 1);
    else if (strcmp(argv[1], "sweep") == 0) run_sweep(argc - 1, argv + 1);
    else { usage(argv[0]); return 1; }
    return 0;
}
