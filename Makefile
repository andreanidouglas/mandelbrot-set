CC=/usr/bin/gcc
CFLAGS=-g -Werror -D _DEFAULT_SOURCE -O0 -D_XOPEN_SOURCE=600 -pthread -pedantic -pedantic-errors -fno-fast-math -fno-builtin -m64 -std=iso9899:1999 -I${PWD} -I${PWD}/../libpng 
OUT=bin
OBJ=$(OUT)/obj
OBJS=$(OBJ)/mandelbrot.o $(OBJ)/img.o


all: pre bin/mandelbrot

bin/mandelbrot: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(OUT)/mandelbrot -lpng -pthread

bin/obj/mandelbrot.o: mandelbrot.c config.h
	$(CC) $(CFLAGS) -c mandelbrot.c -o $(OBJ)/mandelbrot.o -pthread

bin/obj/img.o: ../libpng/img.c
	$(CC) $(CFLAGS) -c ../libpng/img.c -o $(OBJ)/img.o -lpng

pre:
	mkdir -p bin/obj

clean:
	find ./bin -type f -exec rm {} \;