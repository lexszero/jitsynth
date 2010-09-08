#include "common.h"
#include <jit/jit.h>
#include "valstack.h"

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

/* Don't remember why i need this
char *strdup_printf(const char *fmt, ...) {
	char *tmp;
	va_list ap;
	va_start(ap, fmt);
	vasprintf(&tmp, fmt, ap);
	va_end(ap);
	return tmp;
}
*/

jit_context_t jit_context;

typedef struct function_t {
	char *name;
	jit_function_t func;
} function_t;
#include "funclist.h"
funclist_t *funclist;

jit_function_t get_function_by_name(char *name) {
	funclistitem_t *cur;
	LOGF("Looking for %s", name);
	for (cur = funclist->head; cur; cur = cur->next)
		if (strcmp(cur->data->name, name) == 0)
			return cur->data->func;
	LOGF("Not found");
	return NULL;
}

typedef struct track_t {
	jit_function_t instrument;
	double volume;
} track_t;

jit_function_t parse_function(char **str) {
	char *t;

	jit_type_t params[3] = {jit_type_float64, jit_type_nuint, jit_type_nuint},
			   signature = jit_type_create_signature(jit_abi_cdecl, jit_type_float64, params, 3, 1);
	jit_function_t func = jit_function_create(jit_context, signature);
	jit_value_t val_freq = jit_value_get_param(func, 0),
				val_sample = jit_value_get_param(func, 1),
				val_length = jit_value_get_param(func, 2),
				const_rate = jit_value_create_float64_constant(func, jit_type_float64, (double)RATE),
				const_pi = jit_value_create_float64_constant(func, jit_type_float64, (double)M_PI),
				arg1, arg2;

	valstack_t *valstack = stack_new(valstack);

	jit_function_t func1;
	while ((t = token(str)) != NULL) {
		if (STRNEQ(t, "F", 1))
			stack_push(valstack, val_freq);
		else if (STRNEQ(t, "X", 1))
			stack_push(valstack, val_sample);
		else if (STRNEQ(t, "LEN", 3))
			stack_push(valstack, val_length);
		else if (STRNEQ(t, "RATE", 4))
			stack_push(valstack, const_rate);
		else if (STRNEQ(t, "PI", 2))
			stack_push(valstack, const_pi);
		else if (STRNEQ(t, "+", 1))
			stack_push(valstack, jit_insn_add(func, stack_pop(valstack), stack_pop(valstack)));
		else if (STRNEQ(t, "-", 1)) {
			arg1 = stack_pop(valstack);
			arg2 = stack_pop(valstack);
			stack_push(valstack, jit_insn_sub(func, arg2, arg1));
		}
		else if (STRNEQ(t, "*", 1))
			stack_push(valstack, jit_insn_mul(func, stack_pop(valstack), stack_pop(valstack)));
		else if (STRNEQ(t, "/", 1)) {
			arg1 = stack_pop(valstack);
			arg2 = stack_pop(valstack);
			stack_push(valstack, jit_insn_div(func, arg2, arg1));
		}
		else if (STRNEQ(t, "%", 1)) {
			arg1 = stack_pop(valstack);
			arg2 = stack_pop(valstack);
			stack_push(valstack, jit_insn_rem(func, arg2, arg1));
		}
		else if (STRNEQ(t, ">", 1)) {
			arg1 = stack_pop(valstack);
			arg2 = stack_pop(valstack);
			stack_push(valstack, jit_insn_gt(func, arg1, arg2));
		}
		else if (STRNEQ(t, "<", 1)) {
			arg1 = stack_pop(valstack);
			arg2 = stack_pop(valstack);
			stack_push(valstack, jit_insn_lt(func, arg1, arg2));
		}
		else if (STRNEQ(t, "sin", 3))
			stack_push(valstack, jit_insn_sin(func, stack_pop(valstack)));
		else if ((func1 = get_function_by_name(t)) != NULL) {
			arg1 = stack_pop(valstack);
			arg2 = stack_pop(valstack);
			jit_value_t args[3] = {arg1, arg2, val_length};
			LOGF("calling %s", t);
			stack_push(
					valstack,
					jit_insn_call(
						func,
						strdup(t),
						func1,
						NULL,
						args,
						3,
						0));
		}
		else 
			stack_push(valstack, jit_value_create_float64_constant(func, jit_type_float64, atof(t)));
	}

	jit_insn_return(func, stack_pop(valstack));
	jit_function_compile(func);
	
	stack_free(valstack);

	return func;
}

jit_float64 note_freq(int n) {
	return 440*pow(2, n/12.0);
}

jit_nuint note_len(int n) {
	return RATE/(1 << n);
}

int main() {
	char input[1000], *c, *t;

	funclist = funclist_new();

	jit_context = jit_context_create();
	jit_context_build_start(jit_context);

	function_t *temp_func;
	track_t track;
	track.volume = 1;
	while (fgets(input, 1000, stdin)) {
		if (input[0] == '#') continue;

		c = input;
		t = token(&c);
		if (STRNEQ(t, "function", 8)) {
			temp_func = calloc(1, sizeof(function_t));
			temp_func->name = strdup(token(&c));
			LOGF("temp_func->name=%s", temp_func->name);
			temp_func->func = parse_function(&c);
			slist_add(funclist, temp_func);
		}

		if (STRNEQ(t, "track", 5)) {
			t = token(&c);
			track.instrument = get_function_by_name(t);
			LOGF("track.instrument=%p", track.instrument);
		}

		if (STRNEQ(t, "body", 4))
			break;
	}
	jit_context_build_end(jit_context);

	jit_float64 result,
				freq;
	jit_nuint	sample,
				cur_note_len;
	void *args[3] = { &freq, &sample, &cur_note_len};

	int16_t raw_sample;

	int set_note_len = 2;
	cur_note_len = note_len(set_note_len);

	char ch;
	while ((ch = fgetc(stdin)) != EOF) {
		if (ch == '+') {
			set_note_len--;
			cur_note_len = note_len(set_note_len);
		}
		else if (ch == '-') {
			set_note_len++;
			cur_note_len = note_len(set_note_len);
		}
		else if (ch == ' ') {
			raw_sample = 0;
			for (sample = 0; sample < cur_note_len; sample++) 
				write(1, &raw_sample, 2);
		}
		else if (ch >= 'a' && ch <= 'z') {
			freq = note_freq(ch-'a');
			LOGF("note %f %i", freq, cur_note_len);
			for (sample = 0; sample < cur_note_len; sample++) {
				jit_function_apply(track.instrument, args, &result);
				//LOGF("f(%i, %i) = %f", freq, sample, result);
				raw_sample = result*32767;
				write(1, &raw_sample, 2);
			}
		}
	}
	
	slist_free(funclist);
	jit_context_destroy(jit_context);
	return 0;
}
