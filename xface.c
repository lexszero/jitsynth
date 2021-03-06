#include "xface.h"

Display *dis;
xcb_connection_t *xcb_conn;

static void kill_autorepeat(Display *dis) {
	Bool sup = False;
	Bool ret = XkbSetDetectableAutoRepeat(dis, True, &sup);
	assert(sup == True && ret == True);
	XFlush(dis);
}

void *xface() {
	LOGF("xface thread started");
	
	Display *dis = XOpenDisplay(NULL);
	xcb_conn = XGetXCBConnection(dis);
	const xcb_setup_t *xcb_setup = xcb_get_setup(xcb_conn);
	xcb_screen_iterator_t screen_iter = xcb_setup_roots_iterator(xcb_setup);
	xcb_screen_t *xcb_screen = screen_iter.data;
	xcb_window_t window = xcb_generate_id(xcb_conn);
	static const uint32_t xcb_values[] = { XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE };  

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
	
	kill_autorepeat(dis);

	xcb_generic_event_t *ev;
	keydown_t keydown;
	keylistitem_t *ki, *kni;
	while ((ev = xcb_wait_for_event(xcb_conn)) != NULL) {
		switch (ev->response_type & ~0x80) {
			case XCB_KEY_PRESS: ;;
				xcb_key_press_event_t *kp = (xcb_key_press_event_t *)ev;
				LOGF("Pressed %i %i", kp->detail, kp->state);
				
				bool already_pressed = false;
				list_foreach(keylist, ki) {
					if (ki->data.key == kp->detail) {
						already_pressed = true;
						break;
					}
				}
				if (already_pressed) break;
				if (tracklist->head) {
					keydown.key = kp->detail;
					keydown.data.pitem = track_play_functional(&(tracklist->head->data), note_freq(kp->detail-20), note_len_infinity);
					
					list_add_tail(keylist, keylist, keydown);
				}

				break;
			case XCB_KEY_RELEASE: ;;
				xcb_key_release_event_t *kr = (xcb_key_release_event_t *)ev;
				LOGF("Released %i %i", kr->detail, kr->state);
				
				list_foreach_safe(keylist, ki, kni, {
					if (ki->data.key == kr->detail) {
						playing_release(ki->data.data.pitem);
						list_delete(keylist, keylist, ki);
					}
				});

				break;
		}
	}

	xcb_disconnect(xcb_conn);
	LOGF("xface thread finished");
	running = false;
	return NULL;
}

pthread_t xface_thread;

void xface_init() {
	keylist = list_new(keylist);

	if (pthread_create(&xface_thread, NULL, xface, NULL) != 0) {
		LOGF("xface thread creation failed");
		return;
	}
	list_free(keylist, keylist);
}
