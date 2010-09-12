#include "common.h"
#include "function.h"
#include "track.h"
#include <sys/soundcard.h>
#include <sys/ioctl.h>

const jit_nuint note_len_infinity = -1;
track_t *tracks[MAX_TRACK];
int tracks_count;

static int fd;

jit_float64 track_get_sample(track_t *t) {
	jit_float64 result;
	if (track_busy(t)) {
		LOGF("track %p busy", t);
		return 0;
	}
	switch (t->type) {
		case T_NULL: ;;
				result = 0;
				break;
		case T_FUNCTIONAL: ;;
				instrument_functional_t *instr = t->ptr.i_functional;
				if ((t->sample < instr->len) || (instr->len == note_len_infinity)) {
					void *args[3] = { &(instr->freq), &(t->sample), &(instr->len)};
					jit_function_apply(instr->func, args, &result);
				}
				else 
					result = 0;
				break;
		case T_SAMPLER: ;;
				// TODO :)
				result = 0;
				break;
	}
	t->sample++;
	track_unlock(t);
	return result;
}

void * player(void *args) {
	uint16_t raw_sample;
	jit_float64 sample;
	int i;
	LOGF("player thread started");
	while (running) {
		sample = 0;
		for (i = 0; i < tracks_count; i++) {
			sample += track_get_sample(tracks[i]) * tracks[i]->volume;
		}
		raw_sample = sample * 65535;
		write(fd, &raw_sample, 2); 
	}
	LOGF("player thread finished");
	return NULL;
}

pthread_t player_thread;

void init_player() {
	tracks_count = 0;
    int i;
    if ((fd = open("/dev/dsp", O_WRONLY)) == -1) {
        LOGF("/dev/dsp open failed: %i", errno);
		return;
    }

	i = 1;
	ioctl(fd, SNDCTL_DSP_CHANNELS, &i);
	i = AFMT_U16_LE;
	ioctl(fd, SNDCTL_DSP_SETFMT, &i);
	i = RATE;
	ioctl(fd, SNDCTL_DSP_SPEED, &i);

	if (pthread_create(&player_thread, NULL, player, NULL) != 0) {
		LOGF("player thread creation failed");
		return;
	}
}
