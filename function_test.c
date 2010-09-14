#include "common.h"
#include "function.h"

jit_context_t jit_context;

int main(int argc, char **argv) {
	if (argc < 4) {
		LOGF("you must provide 3 arguments to function: F, X, LEN");
		return 0;
	}
	jit_context = jit_context_create();
	jit_context_build_start(jit_context);

	jit_function_t func = parse_function();
	jit_context_build_end(jit_context);
	
	jit_float64 result,
				f = atof(argv[1]);
	jit_nuint x = atoi(argv[2]),
			  len = atoi(argv[3]);
	void *args[3] = {&f, &x, &len};
	
	jit_function_apply(func, args, &result);
	LOGF("f(%f, %i, %i) = %f", f, x, len, result);

	jit_context_destroy(jit_context);

	return 0;
}
