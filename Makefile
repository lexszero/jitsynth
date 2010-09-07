CFLAGS += -ggdb -O0 -Wall
LDFLAGS += -lm -ljit

all: synth

synth: synth.o

synth.o: synth.c common.h valstack.h funclist.h

valstack.h: generic_stack.h
	sed "s/NAME/valstack/g; s/TYPE/jit_value_t/g" generic_stack.h > valstack.h	

funclist.h: generic_slist.h
	sed "s/NAME/funclist/g; s/TYPE/function_t */g" generic_slist.h > funclist.h	

clean:
	rm *.o synth valstack.h funclist.h
