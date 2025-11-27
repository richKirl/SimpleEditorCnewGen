CC=clang-20
CFLAGS=-c -ansi -DSHM
LDFLAGS=-I/usr/include/freetype2
LINKS=-lX11 -lXft
all: test

test: test.o
	$(CC) test.o -o test $(LDFLAGS) $(LINKS)

test.o: test.c
	$(CC) $(CFLAGS) test.c $(LDFLAGS)

clean:
	rm -f *.o test
