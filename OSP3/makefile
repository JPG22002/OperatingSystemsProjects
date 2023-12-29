CC=gcc
CFLAGS=-Wall -g -pthread
LDFLAGS=-lm -pthread
OBJECTS=bitmap.o mandel.o mandelmovie.o

all: mandel mandelmovie

mandel: bitmap.o mandel.o
	$(CC) $(LDFLAGS) -o mandel mandel.o bitmap.o

mandelmovie: mandelmovie.o
	$(CC) $(LDFLAGS) -o mandelmovie mandelmovie.o

bitmap.o: bitmap.c bitmap.h
	$(CC) $(CFLAGS) -c bitmap.c

mandel.o: mandel.c bitmap.h
	$(CC) $(CFLAGS) -c mandel.c

mandelmovie.o: mandelmovie.c
	$(CC) $(CFLAGS) -c mandelmovie.c

clean:
	rm -f *.o mandel mandelmovie mandel*.bmp
