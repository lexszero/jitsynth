CFLAGS += -O3 -Wall -Wextra -std=c99
LDFLAGS += -lm -ljit -lxcb -lX11 -lX11-xcb

all: synth

synth: synth.o function.o player.o track.o util.o xface.o

synth.o: common.h player.h track.h xface.h function.h synth.c

util.o: util.c common.h

function.o: function.c function.h common.h funclist.h

player.o: player.c player.h track.h common.h function.h

track.o: track.c track.h player.h common.h tracklist.h

xface.o: xface.h common.h xface.c

track.h: common.h player.h tracklist.h

player.h: plist.h

xface.h: common.h keylist.h

funclist.h: generic_list.h
	sed "s/NAME/funclist/g; s/TYPE/function_t/g" generic_list.h > funclist.h	

tracklist.h: generic_list.h
	sed "s/NAME/tracklist/g; s/TYPE/track_t/g" generic_list.h > tracklist.h	

plist.h: generic_list.h
	sed "s/NAME/plist/g; s/TYPE/playing_t/g" generic_list.h > plist.h

keylist.h: generic_list.h
	sed "s/NAME/keylist/g; s/TYPE/keydown_t/g" generic_list.h > keylist.h

function_test: function_test.c function.o util.o

.PHONY: clean

clean:
	rm *.o synth funclist.h tracklist.h plist.h keylist.h
