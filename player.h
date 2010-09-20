#ifndef _PLAYER_H
#define _PLAYER_H

extern const jit_nuint note_len_infinity;

typedef struct playing_functional_t {
	jit_float64 freq,
				vol,
				sustain_vol;
	jit_nuint sample,
			  len,
			  release_start;
	enum {
		S_ATTACK,
		S_SUSTAIN,
		S_RELEASE
	} state;
} playing_functional_t;

typedef struct playing_sampler_t {
	jit_nuint sample;
	unsigned id,
			 loop;
	// TODO
} playing_sampler_t;

typedef struct track_t track_t;

typedef struct playing_t {
	pthread_mutex_t mutex;
	track_t *track;
	union {
		playing_functional_t functional;
		playing_sampler_t sampler;
	} state;
} playing_t;

typedef enum {
	S_REALTIME,
	S_PREDEFINED
} track_source;

typedef enum {
	T_FUNCTIONAL,
	T_SAMPLER
} track_type;

typedef struct {
	jit_function_t func,
				   attack,
				   release;
	jit_nuint	attack_len,
				release_len;
} track_param_functional_t;

typedef struct {
	// TODO
	size_t sample_count;
	char **sample_fn;
} track_param_sampler_t;

#include "plist.h"

struct track_t {
	pthread_mutex_t mutex;
	plist_t *plist;
	jit_float64 volume;
	track_source source;
	track_type type;
	union {
		track_param_functional_t p_functional;
		track_param_sampler_t p_sampler;
	} param;
};

#define mutex_lock(t) pthread_mutex_lock(&(t->mutex))
#define mutex_busy(t) (pthread_mutex_trylock(&(t->mutex)) == EBUSY)
#define mutex_unlock(t) pthread_mutex_unlock(&(t->mutex))

extern pthread_t player_thread;
extern void init_player(char *dsp);
extern void destroy_player();

#endif
