CC=gcc
CFLAGS=-Wall -g 
LDFLAGS=-lX11 $(shell pkg-config libpulse --cflags --libs) -lpthread -lmpdclient 


all: barbaloop

%.o: %.c %.h
	$(CC) -c $(CFLAGS) $<

barbaloop.o: barbaloop.c  config.h pa.h

barbaloop: barbaloop.o
	$(CC) $^ $(LDFLAGS) -o $@

clean:
	rm -f *.o barbaloop

install:
	cp -f barbaloop ~/.local/bin/barbaloop

