#include "common.h"
#include "function.h"

#include "funclist.h"
funclist_t *funclist;

#include "valstack.h"

jit_function_t get_function_by_name(char *name) {
	funclistitem_t *cur;
	LOGF("Looking for %s", name);
	for (cur = funclist->head; cur; cur = cur->next)
		if (strcmp(cur->data->name, name) == 0)
			return cur->data->func;
	LOGF("Not found");
	return NULL;
}

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

void add_function(char *name, jit_function_t func) {
	function_t *t = calloc(1, sizeof(function_t));
	t->name = name;
	t->func = func;
	slist_add(funclist, t);
}

void init_function() {
	funclist = funclist_new();
}
