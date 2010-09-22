#define _XOPEN_SOURCE 500

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
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
#define STREQ(a, b) (strcmp(a, b) == 0)

#define mutex_lock(t) { LOGF("locking"); pthread_mutex_lock(&((t).mutex)); }
#define mutex_busy(t) (pthread_mutex_trylock(&((t).mutex)) == EBUSY)
#define mutex_unlock(t) pthread_mutex_unlock(&((t).mutex));

extern bool running;

extern jit_context_t jit_context;

extern char *token();
extern jit_float64 note_freq(int n);
extern jit_nuint note_len(int n);
