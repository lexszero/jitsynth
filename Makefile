TARGET = synth
SRC = synth.c player.c function.o track.c util.c xface.c
CFLAGS += -O0 -ggdb -Wall -Wextra -std=c99 -MD -MP -MF .dep/$(@F).d
LDFLAGS += -lm -ljit -lxcb -lX11 -lX11-xcb

all: $(TARGET)

function_test: function_test.c function.o util.o

CC = gcc
COBJ = $(SRC:.c=.o)

$(COBJ): %.o : %.c
	$(CC) -c $(CFLAGS) $< -o $@ 

.SECONDARY: $(TARGET)
$(TARGET): $(COBJ)
	$(CC) $(CFLAGS) $(COBJ) --output $@ $(LDFLAGS)

-include $(shell mkdir .dep 2>/dev/null) $(wildcard .dep/*)

.PHONY: clean

clean:
	rm -f *.o $(TARGET) $(GENSRC)
	rm -rf .dep
