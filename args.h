/* Minimal --key value / --flag command-line parsing. */
#ifndef ARGS_H
#define ARGS_H
#include <string.h>

static inline const char *arg_str(int argc, char **argv, const char *key, const char *def) {
    for (int i = 0; i < argc - 1; i++) if (strcmp(argv[i], key) == 0) return argv[i + 1];
    return def;
}
static inline int arg_flag(int argc, char **argv, const char *key) {
    for (int i = 0; i < argc; i++) if (strcmp(argv[i], key) == 0) return 1;
    return 0;
}
#endif
