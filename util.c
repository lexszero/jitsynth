#include "common.h"

static char *skip_chars = " \r\n\t";
/* Ugly tokenizer :3 */
char *token() {
	char c;
	c = getchar();
	while ((c != EOF) && (strchr(skip_chars, c) != NULL))
		c = getchar();
	if (c == EOF)
		return NULL;
	char buf[256], *cur = buf;
	while ((c != EOF) && (strchr(skip_chars, c) == NULL)) {
		*(cur++) = c;
		c = getchar();
	}
	*cur = 0;
	//LOGF("token=%s", buf);
	// you should care about freeing memory :3
	return strdup(buf);
}

jit_float64 note_freq(int n) {
	return 440*pow(2, n/12.0);
}

jit_nuint note_len(int n) {
	return RATE/(1 << n);
}
