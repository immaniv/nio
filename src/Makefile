CC=gcc
LIBS=-lrt -lpthread
DFLAGS=-DDEBUG -DDEBUGV

mem : mem.c common.c aio.c debug.c
	$(CC) -o $@ $^ $(LIBS)

dmem : mem.c common.c aio.c debug.c
	$(CC) -g -o $@ $^ -lrt -DDEBUG -DDEBUGV

pta : pthread_affinity.c
	$(CC) -g -o $@ $^ -lrt  -lpthread

