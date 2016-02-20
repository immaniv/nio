LIBS=-lrt -lpthread
INCLUDE=-I .
CC=gcc 
OPTS=-O3 -Wall
DFLAGS=-DDEBUG
SRCS=main.c iothread.c work.c signal.c common.c
AR=ar

all: vwrio d.vwrio libxio.so.1.0.1 libxio.a

vwrio: $(SRCS)
	$(CC) $^ $(LIBS) $(INCLUDE) $(OPTS) -o $@ 
	strip $@

d.vwrio: $(SRCS)
	$(CC) $^ $(LIBS) $(INCLUDE) $(OPTS) $(DFLAGS) -g -o $@ 

libxio.so: iothread.c work.c signal.c common.c aio.c
	$(CC) -fPIC -g -c -Wall $^
	$(CC) -shared -Wl,-soname,libxio.so -o libxio.so iothread.o work.o signal.o common.o aio.o

libxio.a: iothread.c work.c signal.c common.c aio.c
	$(CC) -fPIC -g -c -Wall $^
	$(AR) rcs $@ iothread.o work.o signal.o common.o aio.o
clean: 
	rm -f vwrio d.vwrio *.o *.so
