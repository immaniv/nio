LIBS=-lrt -lpthread
INCLUDE=-I .
CC=gcc 
OPTS=-O3 # -Wall
DFLAGS=-DDEBUG
BSRC=main.c iothread.c work.c signal.c common.c init.c
LSRC=iothread.c work.c signal.c common.c aio.c init.c
AR=ar

all: vwrio d.vwrio libxio.so libxio.a

vwrio: $(BSRC)
	$(CC) $^ $(LIBS) $(INCLUDE) $(OPTS) -o $@ 
	strip $@

d.vwrio: $(BSRC)
	$(CC) $^ $(LIBS) $(INCLUDE) $(OPTS) $(DFLAGS) -g -o $@ 

libxio.so: $(LSRC)
	$(CC) -fPIC -g -c $^
	$(CC) -shared -Wl,-soname,libxio.so -o libxio.so iothread.o work.o signal.o common.o aio.o

libxio.a: $(LSRC)
	$(CC) -fPIC -g -c $^
	$(AR) rcs $@ iothread.o work.o signal.o common.o aio.o
clean: 
	rm -f vwrio d.vwrio *.o *.so
