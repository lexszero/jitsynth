#include "common.h"
#include "function.h"
#include "track.h"

#include <xcb/xcb.h>

bool running;
jit_context_t jit_context;


int main() {

	running = true;
	init_function();
	init_player();

	jit_context = jit_context_create();
	jit_context_build_start(jit_context);
	
	/* This parser sucks, but works. */
	char *t = token();
	bool parser_ok = false;
	enum {
		PC_EXIT,
		PC_MAIN,
		PC_TRACK,
		PC_FUNCTION
	} parser_context = PC_MAIN;
	track_t *tmptrack;
	do {
		if (t[0] == '#') continue;
		parser_ok = false;

		switch (parser_context) {
			case PC_MAIN: ;;
				if (STREQ(t, "function")) {
					parser_context = PC_FUNCTION;
					t = token();
					LOGF("found function %s", t);
					jit_function_t tmpfunc = parse_function();
					if (tmpfunc) 
						add_function(strdup(t), tmpfunc);
					else
						LOGF("function '%s' parse failed", t);
					parser_context = PC_MAIN;
					parser_ok = true;
				}
				else if (STREQ(t, "track")) {
					parser_context = PC_TRACK;
					tmptrack = calloc(1, sizeof(track_t));
					LOGF("adding track, track_count=%i", tracks_count);
					tracks[tracks_count++] = tmptrack;
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
					track_lock(tmptrack);
					tmptrack->volume = atof(t)/100.0;
					track_unlock(tmptrack);
					parser_ok = true;
				}
				else if (STREQ(t, "function")) {
					t = token();
					track_lock(tmptrack);
					tmptrack->type = T_FUNCTIONAL;
					tmptrack->ptr.i_functional = calloc(1, sizeof(instrument_functional_t));
					tmptrack->ptr.i_functional->func = get_function_by_name(t);
					track_unlock(tmptrack);
					parser_ok = true;
				}
				else if (STREQ(t, "attack")) {
					t = token();
					track_lock(tmptrack);
					tmptrack->ptr.i_functional->attack = get_function_by_name(t);
					t = token();
					tmptrack->ptr.i_functional->attack_len = atof(t)*RATE;
					track_unlock(tmptrack);
					parser_ok = true;
				}
				else if (STREQ(t, "release")) {
					t = token();
					track_lock(tmptrack);
					tmptrack->ptr.i_functional->release = get_function_by_name(t);
					t = token();
					tmptrack->ptr.i_functional->release_len = atof(t)*RATE;
					track_unlock(tmptrack);
					parser_ok = true;
				}
				else {
					parser_ok = false;
					parser_context = PC_MAIN;
				}
				break;
		}

		free(t);
		if (parser_context == PC_EXIT)
			break;
		if (parser_ok)
			t = token();
	} while (parser_ok);
	jit_context_build_end(jit_context);

	/* HOLY VERBOSE XCB SHIT! */
	xcb_connection_t *xcb_conn = xcb_connect(NULL, NULL);
	const xcb_setup_t *xcb_setup = xcb_get_setup(xcb_conn);
	xcb_screen_iterator_t screen_iter = xcb_setup_roots_iterator(xcb_setup);
	xcb_screen_t *xcb_screen = screen_iter.data;
	xcb_window_t window = xcb_generate_id(xcb_conn);
	const static uint32_t xcb_values[] = { XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE };  

	xcb_create_window(	xcb_conn,
						XCB_COPY_FROM_PARENT,
						window,
						xcb_screen->root,
						0, 0,
						150, 150,
						1,
						XCB_WINDOW_CLASS_INPUT_OUTPUT,
						xcb_screen->root_visual,
						XCB_CW_EVENT_MASK, xcb_values);
	xcb_map_window(xcb_conn, window);
	xcb_flush(xcb_conn);

	xcb_generic_event_t *ev;
	xcb_keycode_t last_key;
	while ((ev = xcb_wait_for_event(xcb_conn)) != NULL) {
		switch (ev->response_type & ~0x80) {
			case XCB_KEY_PRESS: ;;
				xcb_key_press_event_t *kp = (xcb_key_press_event_t *)ev;
				if (kp->detail != last_key) {
					LOGF("Pressed %i %i", kp->detail, kp->state);	
					track_lock(tracks[0]);
					tracks[0]->ptr.i_functional->freq = note_freq(kp->detail-20);
					tracks[0]->ptr.i_functional->len = note_len_infinity;
					tracks[0]->ptr.i_functional->state = S_ATTACK;
					tracks[0]->sample = 0;
					track_unlock(tracks[0]);
					
					last_key = kp->detail;
				}

				break;
			case XCB_KEY_RELEASE: ;;
				xcb_key_release_event_t *kr = (xcb_key_release_event_t *)ev;
				if (kr->detail == last_key) {
					LOGF("Released %i %i", kr->detail, kr->state);
					track_lock(tracks[0]);
					tracks[0]->ptr.i_functional->state = S_RELEASE;
					tracks[0]->ptr.i_functional->release_start = tracks[0]->sample;
					track_unlock(tracks[0]);
					
					last_key = 0;
				}
				break;

		}

	}
	running = false;

	jit_context_destroy(jit_context);
	xcb_disconnect(xcb_conn);
	return 0;
}
