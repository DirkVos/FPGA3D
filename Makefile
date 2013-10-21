CC=gcc
CFLAGS=-Wall -g
LFLAGS=

SOURCE=$(wildcard src/*.c)
OBJECT=$(patsubst src/%.c,obj/%.o,$(SOURCE))
EXEC="run/exec"

all: exec doxy

doxy:
	doxygen Doxyfile

exec: $(OBJECT)
	$(CC) $(CFLAGS) $(LFLAGS) -o $(EXEC) $^

obj/%.o: src/%.c
	$(CC) $(CFLAGS) -c -o $@ $<
