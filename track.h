#ifndef _TRACK_H
#define _TRACK_H

#include "common.h"
#include "player.h"

#include "tracklist.h"
extern tracklist_t *tracklist;

extern void tracker_init();
extern void tracker_destroy();
extern track_t *track_new(track_type type);
extern void track_set_source(track_t *t, track_source s);
extern void track_set_volume(track_t *t, jit_float64 x);

extern playing_t *track_play_functional(track_t *t, jit_float64 freq, jit_nuint len);
extern playing_t *track_play_sampler(track_t *t, unsigned id, unsigned loop);
/* extern void track_playing_delete(plistitem_t *t); */
extern void track_playing_delete_all(track_t *t);

#endif
