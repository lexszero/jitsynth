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
extern void track_set_function(track_t *t, jit_function_t func);
extern void track_set_attack(track_t *t, jit_function_t func, jit_nuint len);
extern void track_set_release(track_t *t, jit_function_t func, jit_nuint len);

extern plistitem_t *track_play_functional(track_t *t, jit_float64 freq, jit_nuint len);
extern plistitem_t *track_play_sampler(track_t *t, unsigned id, unsigned loop);
/* extern void track_playing_delete(plistitem_t *t); */
extern void playing_release(plistitem_t *t);
extern void track_playing_release_all(track_t *t);
extern void track_playing_delete_all(track_t *t);

#endif
