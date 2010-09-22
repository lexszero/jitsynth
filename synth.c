#include "common.h"
#include "function.h"
#include "player.h"
#include "track.h"
#include "xface.h"

#include <string.h>

bool running;
jit_context_t jit_context;
int main(int argc, char **argv) {

	running = true;
	init_function();
	tracker_init();
	
	if (argc > 1)
		init_player(strdup(argv[1]));
	else
		init_player(NULL);
	
	xface_init();

	jit_context = jit_context_create();
	jit_context_build_start(jit_context);
	
	/* This parser sucks, but works. */
	char *t = token(), *t1;
	bool parser_ok = false;
	enum {
		PC_EXIT,
		PC_MAIN,
		PC_TRACK,
		PC_FUNCTION
	} parser_context = PC_MAIN;
	track_t *tmptrack = NULL;
	do {
		if (t[0] == '#') continue;
		parser_ok = false;

		switch (parser_context) {
			case PC_MAIN: ;;
				if (STREQ(t, "function")) {
					parser_context = PC_FUNCTION;
					parser_ok = true;
				}
				else if (STREQ(t, "track")) {
					parser_context = PC_TRACK;
					tmptrack = track_new(T_FUNCTIONAL); // TODO
					track_set_source(tmptrack, S_REALTIME);
					parser_ok = true;
				}
				else if (STREQ(t, "body")) {
					parser_context = PC_EXIT;
					parser_ok = false;
				}
				break;
			case PC_TRACK: ;;
				if (STREQ(t, "volume")) {
					t = token();
					track_set_volume(tmptrack, atof(t)/100.0);
					parser_ok = true;
				}
				else if (STREQ(t, "function")) {
					if (! tmptrack->param.p_functional.func) {
						t = token();
						track_set_function(tmptrack, get_function_by_name(t));
						parser_ok = true;
					}
					else {
						parser_ok = false;
						parser_context = PC_MAIN;
					}
				}
				else if (STREQ(t, "attack")) {
					t = token();
					t1 = token();
					track_set_attack(tmptrack, get_function_by_name(t), atof(t1)*RATE);
					free(t1);
					parser_ok = true;
				}
				else if (STREQ(t, "release")) {
					t = token();
					t1 = token();
					track_set_release(tmptrack, get_function_by_name(t), atof(t1)*RATE);
					free(t1);
					parser_ok = true;
				}
				else {
					parser_ok = false;
					parser_context = PC_MAIN;
				}
				break;
			case PC_FUNCTION: ;;
				LOGF("found function %s", t);
				jit_function_t tmpfunc = parse_function();
				if (tmpfunc)
					add_function(strdup(t), tmpfunc);
				else
					LOGF("function '%s' parse failed", t);
				parser_context = PC_MAIN;
				parser_ok = true;
				break;
			case PC_EXIT: ;;
				break;
		}

		free(t);
		if (parser_context == PC_EXIT)
			break;
		if (parser_ok)
			t = token();
		if (!t)
			break;
	} while (parser_ok);

//	running = false;
	
	while (1) pause();
	jit_context_build_end(jit_context);
	jit_context_destroy(jit_context);
	return 0;
}
