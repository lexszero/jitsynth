#include "common.h"

/* Ugly tokenizer :3 */
char *token(char **str) {
	while (**str && (**str == ' ' || **str == '\n'))
		(*str)++;
	if (! **str)
		return NULL;
	char *result = *str;
	while (**str &&	(**str != ' ' && **str != '\n')) {
		(*str)++;
	}
	*(*str)++ = 0;
	LOGF("token=%s", result);
	return result;
}

jit_float64 note_freq(int n) {
	return 440*pow(2, n/12.0);
}

jit_nuint note_len(int n) {
	return RATE/(1 << n);
}
