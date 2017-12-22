CC=gcc
CFLAGS= -c

all: invoke-oracle-multithreaded

invoke-oracle-multithreaded: main.o list.o sort.o liboracle_v3.a libhash.a
	$(CC) -o invoke-oracle-multithreaded main.o list.o sort.o -L . -loracle_v3 -lhash -pthread

main.o: sort.h list.h
	$(CC) $(CFLAGS) main.c

list.o: list.h
	$(CC) $(CFLAGS) list.c

sort.o: sort.h
	$(CC) $(CFLAGS) sort.c

clean:
	rm -f *.o

