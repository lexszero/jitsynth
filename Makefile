CFLAGS += -ggdb -O0 -Wall -Wextra -std=c99
LDFLAGS += -lm -ljit -lxcb

all: synth

synth: synth.o function.o track.o util.o

synth.o: synth.c common.h track.h function.h

util.o: util.c common.h

function.o: function.c function.h common.h funclist.h

track.o: track.c track.h common.h function.h

funclist.h: generic_slist.h
	sed "s/NAME/funclist/g; s/TYPE/function_t */g" generic_slist.h > funclist.h	

function_test: function_test.c function.o util.o

.PHONY: clean

clean:
	rm *.o synth funclist.h
