#include "common.h"
#include "track.h"

#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <X11/Xlib-xcb.h>
#include <xcb/xcb.h>

typedef struct {
	xcb_keycode_t key;
	char *cmd;
	union {
		plistitem_t *pitem;
		// TODO
	} data;
} keydown_t;

#include "generic_list.h"
LIST(keylist, keydown_t);
keylist_t *keylist;

extern pthread_t xface_thread;
extern void xface_init();
