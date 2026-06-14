# caos - Computation at the Edge of Chaos

A small C reproduction of the experiments in Chris Langton's *Computation at the
Edge of Chaos: Phase Transitions and Emergent Computation* (Physica D 42, 1990,
the PDF in this directory).

Features:

1. Visualize 1D cellular-automaton space-time evolutions in the terminal.
2. Sweep the `lambda` parameter, generating many random transition functions
   per `lambda`, and measure entropy / mutual information / transient length
   across all of them.

## The lambda parameter

Pick a quiescent state `Sq = 0`. If `n` of the `K^N` transition-table entries map
to `Sq`, then `lambda = (K^N - n) / K^N`. `lambda = 0` is maximally ordered;
`lambda = 1 - 1/K` is maximally disordered. The phase transition ("edge of chaos")
sits in between. Tables are built under strong quiescence (a neighborhood
uniform in `s` maps to `s`) and isotropy (1D: reflection; 2D: rotation), as in
the paper.

- sweep uses the random-table method (each table built from scratch at the
  target `lambda`).
- viz --sweep uses the table-walk-through method (one table progressively
  perturbed toward higher `lambda`).

## Build

```sh
make                 # -> ./caos
```

## Usage

```
caos viz   [--lambda L | --sweep] [--size 128] [--steps 100]
           [--k 4 --n 5] [--init random|patch] [--seed S]

caos sweep [--dim 1|2] [--k 8 --n 5] [--size 64]
           [--lmin 0 --lmax <1-1/K> --lstep 0.01] [--runs 10]
           [--settle 100 --sample 100 --tcap 2000] [--aggregate] [--seed S]
```

Defaults: `viz` -> 1D K=4 N=5; `sweep` -> 2D K=8 N=5.

### Visualization

```sh
./caos viz --lambda 0.05                # ordered: collapses to quiescent/periodic
./caos viz --lambda 0.45 --steps 120    # edge of chaos: propagating structures
./caos viz --lambda 0.75                # chaotic: looks random
./caos viz --sweep                      # walk lambda 0 -> 1-1/K, order -> chaos
./caos viz --lambda 0.45 --init patch   # spread/collapse from a central patch
```

States map to glyphs `" .:-=+*#"` (state 0 = blank). Rows are successive time steps.

### Statistics

`sweep` prints TSV with one row per run (a scatter point, as in the paper):

```
lambda   H   H_norm   mi_temporal   mi_spatial   transient
```

`--aggregate` prints one row per `lambda` with the mean of each column instead.

```sh
# paper-exact 2D K=8 N=5; scale runs up for smoother plots (paper used ~10000)
./caos sweep --dim 2 --k 8 --n 5 --runs 20 > data.tsv

# the same engine in 1D K=4 N=5 (matches the visualization)
./caos sweep --dim 1 --k 4 --n 5 --runs 20 > data1d.tsv
```

## Plotting (gnuplot)

```sh
# fig 6: single-cell entropy vs lambda (note the bimodal gap)
gnuplot -p -e "plot 'data.tsv' u 1:2 pt 7 ps 0.3 title 'H'"

# fig 11/12: temporal mutual information vs lambda (jump then slow decay)
gnuplot -p -e "plot 'data.tsv' u 1:4 pt 7 ps 0.3 title 'I(cell;cell@t+1)'"

# fig 14: mutual information vs normalized entropy (single peak at intermediate H)
gnuplot -p -e "plot 'data.tsv' u 3:4 pt 7 ps 0.3 title 'MI vs H'"

# fig 3: transient length vs lambda (critical slowing down)
gnuplot -p -e "set logscale y; plot 'data.tsv' u 1:6 pt 7 ps 0.3 title 'transient'"
```

Replace `-p` (persistent window) with `-e "set term dumb;"` for an in-terminal plot.

## Files

| file              | role                                                        |
|-------------------|-------------------------------------------------------------|
| `ca.{h,c}`        | engine: lambda tables (both methods), constraints, stepping |
| `stats.{h,c}`     | entropy, mutual information, transient detection            |
| `viz.c`           | 1D ASCII space-time visualization                           |
| `sweep.c`         | lambda sweep, TSV output                                    |
| `main.c`          | subcommand dispatch                                         |
| `rng.h` / `args.h`| xorshift PRNG / CLI parsing                                 |
