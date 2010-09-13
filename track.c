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
	jit_float64 result, result1;
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
				void *args[3] = { &(instr->freq), &(t->sample), &(instr->len)};
				switch (instr->state) {
					case S_MUTE: ;;
						result = 0;
						break;
					case S_ATTACK: ;;
						args[2] = &(instr->attack_len);
						jit_function_apply(instr->attack, args, &result1);
						result = result1;
						if (t->sample > instr->attack_len)
							instr->state = S_SUSTAIN;
					case S_RELEASE: ;;
						args[2] = &(instr->release_len);
						jit_function_apply(instr->release, args, &result1);
						result = result1;
						t->release_sample++;
						if (t->release_sample > instr->release_len)
							instr->state = S_MUTE;
					default: ;;
						args[2] = &(instr->len);
						jit_function_apply(instr->func, args, &result1);
						result *= result1;
						break;
				}
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
	const size_t buf_size = 256;
	uint16_t buf[buf_size];
	jit_float64 sample;
	int i, j;
	LOGF("player thread started");
	while (running) {
		for (j = 0; j < buf_size; j++) {
			sample = 0;
			for (i = 0; i < tracks_count; i++) {
				sample += track_get_sample(tracks[i]) * tracks[i]->volume;
			}
			buf[j] = sample * 65535;
		}
		write(fd, &buf, sizeof(uint16_t)*buf_size); 
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
