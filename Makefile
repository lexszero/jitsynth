TARGET = synth
SRC = synth.c player.c function.o track.c util.c xface.c
GENSRC = funclist.h tracklist.h plist.h keylist.h
CFLAGS += -O0 -ggdb -Wall -Wextra -std=c99 -MD -MP -MF .dep/$(@F).d
LDFLAGS += -lm -ljit -lxcb -lX11 -lX11-xcb

all: $(TARGET)

function_test: function_test.c function.o util.o

CC = gcc
COBJ = $(SRC:.c=.o)

$(COBJ): %.o : %.c $(GENSRC)
	$(CC) -c $(CFLAGS) $< -o $@ 

.SECONDARY: $(TARGET)
$(TARGET): $(COBJ)
	$(CC) $(CFLAGS) $(COBJ) --output $@ $(LDFLAGS)

-include $(shell mkdir .dep 2>/dev/null) $(wildcard .dep/*)

funclist.h: generic_list.h
	sed "s/NAME/funclist/g; s/TYPE/function_t/g" generic_list.h > funclist.h	

tracklist.h: generic_list.h
	sed "s/NAME/tracklist/g; s/TYPE/track_t/g" generic_list.h > tracklist.h	

plist.h: generic_list.h
	sed "s/NAME/plist/g; s/TYPE/playing_t/g" generic_list.h > plist.h

keylist.h: generic_list.h
	sed "s/NAME/keylist/g; s/TYPE/keydown_t/g" generic_list.h > keylist.h


.PHONY: clean

clean:
	rm -f *.o $(TARGET) $(GENSRC)
	rm -rf .dep
