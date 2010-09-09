#include "common.h"
#include "function.h"
#include "track.h"
#include <ncurses.h>

bool running;
jit_context_t jit_context;

int main() {
	char input[1000], *c, *t;

	running = true;
	init_function();
	init_player();
	jit_context = jit_context_create();
	jit_context_build_start(jit_context);

	while (fgets(input, 1000, stdin)) {
		if (input[0] == '#') continue;

		c = input;
		t = token(&c);
		if (STRNEQ(t, "function", 8)) {
			t = token(&c);
			add_function(strdup(t), parse_function(&c));
		}

		if (STRNEQ(t, "track", 5)) {
			track_t *track = calloc(1, sizeof(track_t));
			t = token(&c);
			track->volume = atof(t)/100.0;
			t = token(&c);
			if (STRNEQ(t, "function", 8)) {
				t = token(&c);
				track->type = T_FUNCTIONAL;
				instrument_functional_t *fi = calloc(1, sizeof(instrument_functional_t));
				fi->func = get_function_by_name(t);
				track->i_functional = fi;
				tracks[tracks_count++] = track;
				LOGF("adding track, track_count=%i", tracks_count);
			}
			else 
				LOGF("track type '%s' not implemented", t);
		}

		if (STRNEQ(t, "body", 4))
			break;
	}
	jit_context_build_end(jit_context);

	// ACHTUNG! STUBS!
	char ch;
	initscr();
	cbreak();
	timeout(-1);
	while ((ch = getch()) != EOF) {
		LOGF("ch=%c", ch);
		track_lock(tracks[0]);
		LOGF("locked");
		tracks[0]->i_functional->freq = note_freq(ch-'0');
		tracks[0]->i_functional->len = note_len(4);
		tracks[0]->sample = 0;
		track_unlock(tracks[0]);
		LOGF("unlocked");
	}
	running = false;
	jit_context_destroy(jit_context);
	return 0;
}
