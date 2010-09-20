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
	track_t t;
	memset(&t, 0, sizeof(track_t));
	t.type = type;
	t.plist = list_new(plist);
	return &(list_add_tail(tracklist, tracklist, t)->data);
}

/* this two mutator functions should be codogenerated, thou. ROBOWORKZ */
void track_set_source(track_t *t, track_source s) {
	mutex_lock(*t);
	t->source = s;
	mutex_unlock(*t);
}

void track_set_volume(track_t *t, jit_float64 x) {
	mutex_lock(*t);
	t->volume = x;
	mutex_unlock(*t);
}

void track_set_function(track_t *t, jit_function_t func) {
	if (t->type != T_FUNCTIONAL) {
		LOGF("wrong track type");
		return;
	}
	mutex_lock(*t);
	t->param.p_functional.func = func;
	mutex_unlock(*t);
}

void track_set_attack(track_t *t, jit_function_t func, jit_nuint len) {
	if (t->type != T_FUNCTIONAL) {
		LOGF("wrong track type");
		return;
	}
	mutex_lock(*t);
	t->param.p_functional.attack = func;
	t->param.p_functional.attack_len = len;
	mutex_unlock(*t);
}

void track_set_release(track_t *t, jit_function_t func, jit_nuint len) {
	if (t->type != T_FUNCTIONAL) {
		LOGF("wrong track type");
		return;
	}
	mutex_lock(*t);
	t->param.p_functional.release = func;
	t->param.p_functional.release_len = len;
	mutex_unlock(*t);
}
/* end of ROBOWORKZ */

playing_t *track_play_functional(track_t *t, jit_float64 freq, jit_nuint len) {
	if (t->type != T_FUNCTIONAL) {
		LOGF("Trying to play functional in incomplatible track");
		return NULL;
	}
	mutex_lock(*t);
	playing_t p, *pp;
	memset(&p, 0, sizeof(playing_t));
	p.track = t;
	p.state.functional.state = S_ATTACK;
	p.state.functional.sample = 0;
	p.state.functional.freq = freq;
	p.state.functional.len = len;
	pp = &(list_add_tail(plist, t->plist, p)->data);
	mutex_unlock(*t);
	return pp;
}

playing_t *track_play_sampler(track_t *t, unsigned id, unsigned loop) {
	if (t->type != T_SAMPLER) {
		LOGF("Trying to play sampler in incomplatible track");
		return NULL;
	}
	mutex_lock(*t);
	playing_t p, *pp;
	memset(&p, 0, sizeof(playing_t));
	p.track = t;
	p.state.sampler.id = id;
	p.state.sampler.loop = loop;
	pp = &(list_add_tail(plist, t->plist, p)->data);
	mutex_unlock(*t);
	return pp;
}

void track_playing_release_all(track_t *t) {
	if (t->type != T_FUNCTIONAL) {
		LOGF("releasing not implemented");
		return;
	}
	mutex_lock(*t);
	plistitem_t *cur;
	list_foreach(t->plist, cur) {
		if (cur->data.state.functional.state != S_RELEASE) {
			cur->data.state.functional.state = S_RELEASE;
			cur->data.state.functional.release_start = cur->data.state.functional.sample;
		}
	}
	mutex_unlock(*t);
}

void track_playing_delete_all(track_t *t) {
	plistitem_t *cur;
	for (cur = t->plist->head; cur; cur = cur->next) {
		list_delete(plist, t->plist, cur);
	}
}
