extern const jit_nuint note_len_infinity;

typedef struct instrument_functional_t {
	jit_function_t func;
	jit_float64 freq;
	jit_nuint len,
			  default_len;
} instrument_functional_t;

typedef struct instrument_sampler_t {
	int count;
	jit_function_t *func;
} instrument_sampler_t;

typedef struct track_t {
	pthread_mutex_t mutex;
	jit_nuint sample;
	jit_float64 volume;
	enum {
		T_NULL,
		T_FUNCTIONAL,
		T_SAMPLER,
	} type;
	union {
		void *i_nothing;
		instrument_functional_t *i_functional;
		instrument_sampler_t *i_sampler; // TODO :)
	};
} track_t;

#define MAX_TRACK 100
extern track_t *tracks[MAX_TRACK];
extern int tracks_count;

#define track_lock(t) pthread_mutex_lock(&(t->mutex))
#define track_busy(t) (pthread_mutex_trylock(&(t->mutex)) == EBUSY)
#define track_unlock(t) pthread_mutex_unlock(&(t->mutex))
extern jit_float64 track_get_sample(track_t *t);

extern pthread_t player_thread;
extern void init_player();
