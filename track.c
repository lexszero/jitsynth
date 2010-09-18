#include "common.h"
#include "function.h"
#include "track.h"
#include <sys/soundcard.h>
#include <sys/ioctl.h>

const jit_nuint note_len_infinity = -1;
track_t *tracks[MAX_TRACK];
unsigned tracks_count;

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
				void *args[3] = { &(instr->freq), &(t->sample), &(instr->len)};
				if (instr->state == S_ATTACK) {
					if (instr->attack) {
						args[1] = &(t->sample);
						args[2] = &(instr->attack_len);
						jit_function_apply(instr->attack, args, &(instr->vol));
						if (t->sample >= instr->attack_len) {
							LOGF("sustain vol = %f", instr->vol);
							instr->sustain_vol = instr->vol;
							instr->state = S_SUSTAIN;
						}
					}
					else
						instr->state = S_SUSTAIN;
				}

				if (instr->state == S_RELEASE) {
					if (instr->release) {
						jit_nuint release_sample = t->sample - instr->release_start;
						args[1] = &(release_sample);
						args[2] = &(instr->release_len);
						jit_function_apply(instr->release, args, &(result));
						instr->vol = instr->sustain_vol*result;
						if (release_sample >= instr->release_len) {
							LOGF("mute1");
							instr->state = S_MUTE;
						}
					}
					else {
						LOGF("mute2");
						instr->state = S_MUTE;
					}
				}

				if (instr->state != S_MUTE) {
					args[1] = &(t->sample);
					args[2] = &(instr->len);
					jit_function_apply(instr->func, args, &result);
					result *= instr->vol;
				}
				else
					result = 0;

				break;

		case T_SAMPLER: ;;
				// TODO :)
				result = 0;
				break;
		
		default: ;;
	}
	t->sample++;
	track_unlock(t);
	return result;
}

void * player(void *args) {
	const size_t buf_size = 256;
	uint16_t buf[buf_size];
	jit_float64 sample;
	unsigned i, j;
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
	close(fd);
	return NULL;
}

pthread_t player_thread;

void init_player(char *dsp) {
	tracks_count = 0;
    int i;
	if (dsp && STREQ(dsp, "-")) {
		LOGF("output to stdout");
		fd = 1;
	}
	else {
		if (dsp == NULL)
			dsp = strdup("/dev/dsp");
		LOGF("dsp %s", dsp);
		if ((fd = open(dsp, O_WRONLY)) == -1) {
    	    LOGF("%s open failed: %i", dsp, errno);
			return;
		}

		i = 1;
		ioctl(fd, SNDCTL_DSP_CHANNELS, &i);
		i = AFMT_U16_LE;
		ioctl(fd, SNDCTL_DSP_SETFMT, &i);
		i = RATE;
		ioctl(fd, SNDCTL_DSP_SPEED, &i);
	}

	if (pthread_create(&player_thread, NULL, player, NULL) != 0) {
		LOGF("player thread creation failed");
		return;
	}
}
