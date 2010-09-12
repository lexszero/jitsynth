typedef struct function_t {
	char *name;
	jit_function_t func;
} function_t;

extern void init_function();
extern jit_function_t get_function_by_name(char *name);
extern jit_function_t parse_function();
extern void add_function(char *name, jit_function_t func);
