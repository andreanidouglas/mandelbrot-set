CC=/usr/bin/gcc
CFLAGS=-g -Wall -Wextra -Werror -D _DEFAULT_SOURCE -O0 -D_XOPEN_SOURCE=600 -pthread -pedantic -pedantic-errors -fno-fast-math -fno-builtin -m64 -std=iso9899:1999 -I${PWD}  -Iraylib/src
OUT=bin
OBJ=$(OUT)/obj
OBJS=$(OBJ)/mandelbrot.o $(OBJ)/queue.o   


all: pre raylib bin/mandelbrot

bin/mandelbrot: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(OUT)/mandelbrot raylib/src/libraylib.a -lpthread -lm

bin/obj/mandelbrot.o: mandelbrot.c config.h
	$(CC) $(CFLAGS) -c mandelbrot.c -o $(OBJ)/mandelbrot.o 

bin/obj/queue.o: queue.c
	$(CC) $(CFLAGS) -c queue.c -o $(OBJ)/queue.o 

raylib: raylib/src/raylib.h
	$(MAKE) -C raylib/src PLATFORM=PLATFORM_DESKTOP

pre: 
	mkdir -p bin/obj

clean:
	find ./bin -type f -exec rm {} \;
