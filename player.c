#include "common.h"
#include "function.h"
#include "track.h"
#include <sys/time.h>
#include <sys/soundcard.h>
#include <sys/ioctl.h>

const jit_nuint note_len_infinity = -1;

static int fd;

jit_float64 get_sample(plistitem_t *pt) {
	jit_float64 result = 0;
	playing_t *t = &(pt->data);
	if (mutex_busy(*t)) {
		LOGF("playing %p busy", t);
		return 0;
	}
	switch (t->track->type) {
		case T_FUNCTIONAL: ;;
				playing_functional_t *fst = (playing_functional_t *) &(t->state);
				track_param_functional_t *fp = (track_param_functional_t *) &(t->track->param);
				void *args[3] = { &(fst->freq), &(fst->sample), &(fst->len)};
				if (fst->state == S_ATTACK) {
					if (fp->attack) {
						args[1] = &(fst->sample);
						args[2] = &(fp->attack_len);
						jit_function_apply(fp->attack, args, &(fst->vol));
						if (fst->sample >= fp->attack_len) {
							LOGF("sustain vol = %f", fst->vol);
							fst->sustain_vol = fst->vol;
							fst->state = S_SUSTAIN;
						}
					}
					else
						fst->state = S_SUSTAIN;
				}

				if (fst->state == S_RELEASE) {
					if (fp->release) {
						jit_nuint release_sample = fst->sample - fst->release_start;
						args[1] = &(release_sample);
						args[2] = &(fp->release_len);
						jit_function_apply(fp->release, args, &(result));
						fst->vol = fst->sustain_vol*result;
						if (release_sample >= fp->release_len)
							t->delete_me = true;
					}
					else
						t->delete_me = true;
				}

				args[1] = &(fst->sample);
				args[2] = &(fst->len);
				jit_function_apply(fp->func, args, &result);
				result *= fst->vol;

				fst->sample++;

				break;

		case T_SAMPLER: ;;
				// TODO :)
				result = 0;
				break;
		
		default: ;;
	}
	mutex_unlock(*t);
	return result;
}

void * player() {
	const size_t buf_size = 256;
	uint16_t buf[buf_size];
	jit_float64 sample, track_sample;
	unsigned j;
	LOGF("player thread started");
	tracklistitem_t *ti;
	plistitem_t *pi, *pni;
#ifdef DEBUG_PROFILE
	struct timeval tv1, tv2;
	unsigned usec;
#endif
	while (running) {
#ifdef DEBUG_PROFILE
		gettimeofday(&tv1, NULL);
#endif
		for (j = 0; j < buf_size; j++) {
			sample = 0;
			list_foreach(tracklist, ti) {
				if (mutex_busy(ti->data)) {
					LOGF("track %p busy", ti);
					continue;
				}
				else {
					track_sample = 0;
					list_foreach_safe(ti->data.plist, pi, pni, {
						track_sample += get_sample(pi);
						if (pi->data.delete_me) {
							LOGF("deleting finished playing");
							list_delete(plist, ti->data.plist, pi);
						}
					});
					sample += track_sample * ti->data.volume;
					mutex_unlock(ti->data);
				}
			}
			buf[j] = sample * 65535;
		}
		if (write(fd, &buf, sizeof(uint16_t)*buf_size) == 0) {
			LOGF("Write failed");
			break;
		}

#ifdef DEBUG_PROFILE
		gettimeofday(&tv2, NULL);
		usec = tv2.tv_usec - tv1.tv_usec;
		if (usec > 1000000/RATE*buf_size)
			LOGF("Get out of realtime: %u usec", usec);
#endif
	}
	LOGF("player thread finished");
	close(fd);
	return NULL;
}

pthread_t player_thread;

void init_player(char *dsp) {
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

void destroy_player() {
	LOGF("destroying player");
}


