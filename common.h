#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include <jit/jit.h>
#include <pthread.h>

#define RATE 44100

#define LOGF(fmt, args...) fprintf(stderr, "%s:%i %s(): " fmt "\n", __FILE__, __LINE__, __func__,  ##args)
#define STRNEQ(a, b, n) (strncmp(a, b, n) == 0)

extern bool running;

extern jit_context_t jit_context;

extern char *token(char **str);
extern jit_float64 note_freq(int n);
extern jit_nuint note_len(int n);
