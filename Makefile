all:  peer tracker

peer: peer.c
	gcc -o peer peer.c

tracker: tracker.c
	gcc -o tracker tracker.c -lpthread

clean:
	rm -f peer tracker *.o *~ core
