CC=gcc
CFLAGS=-Wall -Werror `pkg-config --cflags x11`
LDFLAGS=`pkg-config --libs x11`

all: barbaloop.c config.h
	$(CC) $(CFLAGS) $(LDFLAGS) -o barbaloop barbaloop.c

clean:
	rm -f *.o barbaloop

install:
	cp -f barbaloop ~/.local/bin/barbaloop

