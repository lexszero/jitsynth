#include "common.h"
#include "function.h"
#include "track.h"

#include <xcb/xcb.h>

bool running;
jit_context_t jit_context;


int main() {
	char *t;

	running = true;
	init_function();
	init_player();

	jit_context = jit_context_create();
	jit_context_build_start(jit_context);

	while ((t = token()) != NULL) {
		if (t[0] == '#') continue;

		if (STRNEQ(t, "function", 8)) {
			t = token();
			LOGF("found function %s", t);
			add_function(strdup(t), parse_function());
		}

		if (STRNEQ(t, "track", 5)) {
			track_t *track = calloc(1, sizeof(track_t));
			t = token();
			track->volume = atof(t)/100.0;
			t = token();
			if (STRNEQ(t, "function", 8)) {
				t = token();
				track->type = T_FUNCTIONAL;
				instrument_functional_t *fi = calloc(1, sizeof(instrument_functional_t));
				fi->func = get_function_by_name(t);
				track->ptr.i_functional = fi;
				tracks[tracks_count++] = track;
				LOGF("adding track, track_count=%i", tracks_count);
			}
			else 
				LOGF("track type '%s' not implemented", t);
		}

		if (STRNEQ(t, "body", 4))
			break;
		free(t);
	}
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
	while ((ev = xcb_wait_for_event(xcb_conn)) != NULL) {
		switch (ev->response_type & ~0x80) {
			case XCB_KEY_PRESS: ;;
				xcb_key_press_event_t *kp = (xcb_key_press_event_t *)ev;
				LOGF("Pressed %i", kp->detail);
	
				track_lock(tracks[0]);
				LOGF("locked");
				tracks[0]->ptr.i_functional->freq = note_freq(kp->detail-10);
				tracks[0]->ptr.i_functional->len = note_len_infinity;
				tracks[0]->sample = 0;
				track_unlock(tracks[0]);
				LOGF("unlocked");

				break;
			case XCB_KEY_RELEASE: ;;
				xcb_key_release_event_t *kr = (xcb_key_release_event_t *)ev;
				LOGF("Released %i", kr->detail);
	
				track_lock(tracks[0]);
				LOGF("locked");
				tracks[0]->ptr.i_functional->len = 0;
				tracks[0]->sample = 0;
				track_unlock(tracks[0]);
				LOGF("unlocked");

				break;

		}

	}
	running = false;

	jit_context_destroy(jit_context);
	xcb_disconnect(xcb_conn);
	return 0;
}
