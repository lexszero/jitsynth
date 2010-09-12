CFLAGS += -ggdb -O0 -Wall #-std=c99
LDFLAGS += -lm -ljit -lcurses

all: synth

synth: synth.o function.o track.o util.o

synth.o: synth.c common.h

util.o: util.c common.h

function.o: function.c function.h common.h funclist.h

track.o: track.c track.h common.h function.h

funclist.h: generic_slist.h
	sed "s/NAME/funclist/g; s/TYPE/function_t */g" generic_slist.h > funclist.h	

clean:
	rm *.o synth funclist.h
