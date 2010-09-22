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

#include "keylist.h"
keylist_t *keylist;

extern void xface_init();
extern void xface_main();
extern void xface_destroy();