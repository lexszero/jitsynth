CFLAGS += -ggdb -O0 -Wall -Wextra -std=c99
LDFLAGS += -lm -ljit -lxcb -lX11 -lX11-xcb

all: synth

synth: synth.o function.o player.o util.o

synth.o: synth.c common.h player.h function.h

util.o: util.c common.h

function.o: function.c function.h common.h funclist.h

player.o: player.c player.h track.h common.h function.h

track.o: track.c track.h player.h common.h tracklist.h plist.h

player.h: tracklist.h plist.h

funclist.h: generic_list.h
	sed "s/NAME/funclist/g; s/TYPE/function_t */g" generic_list.h > funclist.h	

tracklist.h: generic_list.h
	sed "s/NAME/tracklist/g; s/TYPE/track_t */g" generic_list.h > tracklist.h	

plist.h: generic_list.h
	sed "s/NAME/plist/g; s/TYPE/playing_t */g" generic_list.h > plist.h

function_test: function_test.c function.o util.o

.PHONY: clean

clean:
	rm *.o synth funclist.h tracklist.h plist.h
