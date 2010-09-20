#include "common.h"
#include "track.h"

tracklist_t *tracklist;

void tracker_init() {
	tracklist = list_new(tracklist);
}

void tracker_destroy() {
	/* TODO: carefully free() every piece of memory */
	list_free(tracklist, tracklist);
}

track_t *track_new(track_type type) {
	track_t *t = calloc(1, sizeof(track_t));
	t->type = type;
	t->plist = list_new(plist);
	list_add_tail(tracklist, tracklist, t);
	return t;
}

/* this two mutator functions should be codogenerated, thou. ROBOWORKZ */
void track_set_source(track_t *t, track_source s) {
	track_lock(t);
	t->source = s;
	track_unlock(t);
}

void track_set_volume(track_t *t, jit_float64 x) {
	track_lock(t);
	t->volume = x;
	track_unlock(t);
}
/* end of ROBOWORKZ */

playing_t *track_play_functional(track_t *t, jit_float64 freq, jit_nuint len) {
	if (t->type != T_FUNCTIONAL) {
		LOGF("Trying to play functional in incomplatible track");
		return NULL;
	}
	playing_t *p = calloc(1, sizeof(playing_t));
	p->track = t;
	p->state.functional.freq = freq;
	p->state.functional.len = len;
	list_add_tail(plist, t->plist, p);
	return p;
}

playing_t *track_play_sampler(track_t *t, unsigned id, unsigned loop) {
	if (t->type != T_SAMPLER) {
		LOGF("Trying to play sampler in incomplatible track");
		return NULL;
	}
	playing_t *p = calloc(1, sizeof(playing_t));
	p->track = t;
	p->state.sampler.id = id;
	p->state.sampler.loop = loop;
	list_add_tail(plist, t->plist, p);
	return p;
}

void track_playing_delete_all(track_t *t) {
	plistitem_t *cur;
	for (cur = t->plist->head; cur; cur = cur->next) {
		list_delete(plist, t->plist, cur);
	}
}
