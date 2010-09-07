#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include <stdint.h>

#define RATE 44100

#define LOGF(fmt, args...) fprintf(stderr, "%s:%i %s(): " fmt "\n", __FILE__, __LINE__, __func__,  ##args)
#define STRNEQ(a, b, n) (strncmp(a, b, n) == 0)
