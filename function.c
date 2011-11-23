#include "common.h"
#include "function.h"

#include "generic_list.h"
LIST(funclist, function_t);
funclist_t *funclist;

jit_context_t jit_context;

jit_function_t get_function_by_name(char *name) {
	funclistitem_t *cur;
	LOGF("Looking for %s", name);
	for (cur = funclist->head; cur; cur = cur->next)
		if (STREQ(cur->data.name, name))
			return cur->data.func;
	LOGF("Not found");
	return NULL;
}

typedef struct op_t {
	enum {
		OP_CALL = -1,
		OP_NULLARY = 0,
		OP_UNARY = 1,
		OP_BINARY = 2,
		OP_TERNARY = 3
	} nargs;
	jit_value_t (*insn)();
} op_t;

/*
#define OPS_COUNT 11
static const ops[OPS_COUNT] = {
	// nargs		func
	{ OP_CALL,		NULL },
	{ OP_BINARY,	&jit_insn_add},
	{ OP_BINARY,	&jit_insn_sub},
	{ OP_BINARY,	&jit_insn_mul},
	{ OP_BINARY,	&jit_insn_div},
	{ OP_BINARY,	&jit_insn_rem},
	{ OP_BINARY,	&jit_insn_gt},
	{ OP_BINARY,	&jit_insn_lt},
	{ OP_UNARY,		&jit_insn_sin},
	{ OP_BINARY,	&jit_insn_gt},
	{ OP_BINARY,	&jit_insn_lt}};
*/

static jit_value_t const_rate,
				   const_pi,
				   val_freq,
				   val_sample,
				   val_length;


static jit_value_t parse_recursive(jit_function_t func) {
	jit_value_t arg1, arg2, result;
	jit_function_t func1;

	double val;
	char *t = token();

	// Somebody, do something with this!
	// It's awful monkeycoding, but I'm too lazy to rewrite it :3
	if (STREQ(t, "F"))
		result = val_freq;
	else if (STREQ(t, "X"))
		result = jit_insn_convert(func, val_sample, jit_type_float64, 0);
	else if (STREQ(t, "LEN"))
		result = jit_insn_convert(func, val_length, jit_type_float64, 0);
	else if (STREQ(t, "RATE"))
		result = const_rate;
	else if (STREQ(t, "PI"))
		result = const_pi;
	else if (STREQ(t, "+"))
		result = jit_insn_add(func, parse_recursive(func), parse_recursive(func));
	else if (STREQ(t, "-")) {
		arg1 = parse_recursive(func);
		arg2 = parse_recursive(func);
		result = jit_insn_sub(func, arg1, arg2);
	}
	else if (STREQ(t, "*"))
		result = jit_insn_mul(func, parse_recursive(func), parse_recursive(func));
	else if (STREQ(t, "/")) {
		arg1 = parse_recursive(func);
		arg2 = parse_recursive(func);
		result = jit_insn_div(func, arg1, arg2);
	}
	else if (STREQ(t, "%")) {
		arg1 = parse_recursive(func);
		arg2 = parse_recursive(func);
		result = jit_insn_rem(func, arg1, arg2);
	}
	else if (STREQ(t, ">")) {
		arg1 = parse_recursive(func);
		arg2 = parse_recursive(func);
		result = jit_insn_gt(func, arg1, arg2);
	}
	else if (STREQ(t, "<")) {
		arg1 = parse_recursive(func);
		arg2 = parse_recursive(func);
		result = jit_insn_lt(func, arg1, arg2);
	}
	else if (STREQ(t, "if")) {
		jit_value_t tmpval = jit_value_create(func, jit_type_float64);
		jit_label_t lb_false = jit_label_undefined,
					lb_end = jit_label_undefined;
		jit_insn_branch_if_not(func, jit_insn_to_bool(func, parse_recursive(func)), &lb_false);
		jit_insn_store(func, tmpval, parse_recursive(func));
		jit_insn_branch(func, &lb_end);
		jit_insn_label(func, &lb_false);
		jit_insn_store(func, tmpval, parse_recursive(func));
		jit_insn_label(func, &lb_end);
		result = jit_insn_load(func, tmpval);
	}
	else if (STREQ(t, "sin"))
		result = jit_insn_sin(func, parse_recursive(func));
	else if (sscanf(t, "%lf", &val) == 1)
		result = jit_value_create_float64_constant(func, jit_type_float64, val);
	else if ((func1 = get_function_by_name(t)) != NULL) {
			arg1 = parse_recursive(func);
			arg2 = parse_recursive(func);
			jit_value_t args[3] = {arg1, arg2, val_length};
			result = jit_insn_call(	
						func,
						t,
						func1,
						NULL,
						args,
						3,
						0);
	}
	else {
		LOGF("Unexpected token '%s'", t);
		result = NULL;
	}
	free(t);
	return result;
}

jit_function_t parse_function() {
	jit_type_t params[3] = {jit_type_float64, jit_type_nuint, jit_type_nuint},
			   signature = jit_type_create_signature(jit_abi_cdecl, jit_type_float64, params, 3, 1);
	jit_function_t func = jit_function_create(jit_context, signature);
	const_rate = jit_value_create_float64_constant(func, jit_type_float64, (double)RATE),
	const_pi = jit_value_create_float64_constant(func, jit_type_float64, (double)M_PI),

	val_freq = jit_value_get_param(func, 0);
	val_sample = jit_value_get_param(func, 1);
	val_length = jit_value_get_param(func, 2);
	
	jit_insn_return(func, parse_recursive(func));
	jit_function_compile(func);

	return func;
}

void add_function(char *name, jit_function_t func) {
	function_t t;
	memset(&t, 0, sizeof(function_t));
	t.name = name;
	t.func = func;
	list_add_tail(funclist, funclist, t);
}

void function_init() {
	funclist = list_new(funclist);
	jit_context = jit_context_create();
	jit_context_build_start(jit_context);
}

void function_destroy() {
	list_free(funclist, funclist);
	jit_context_build_end(jit_context);
	jit_context_destroy(jit_context);
}
