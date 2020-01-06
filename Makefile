CC=cc

all:
	$(CC) -o barbaloop barbaloop.c -lpthread

clean:
	rm *.o barbaloop
