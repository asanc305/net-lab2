all:  peer tracker

peer: peer.c
	gcc -o peer peer.c

myftptracker: tracker.c
	gcc -o tracker tracker.c

clean:
	rm -f peer tracker *.o *~ core
