CC      ?= cc
CFLAGS  ?= -O2 -std=c11 -Wall -Wextra
SRC      = main.c ca.c stats.c viz.c sweep.c
HDR      = ca.h stats.h rng.h args.h

caos: $(SRC) $(HDR)
	$(CC) $(CFLAGS) -o $@ $(SRC) -lm

clean:
	rm -f caos

.PHONY: clean
