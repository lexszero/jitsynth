typedef struct function_t {
	char *name;
	jit_function_t func;
	size_t nargs;
} function_t;

extern void function_init();
extern void function_destroy();
extern jit_function_t get_function_by_name(char *name);
extern jit_function_t parse_function();
extern void add_function(char *name, jit_function_t func);
