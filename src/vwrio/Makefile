LIBS=-lrt -lpthread
INCLUDE=-I .
CC=gcc 
OPTS=-O3
DFLAGS=-DDEBUG

all: vwrio d.vwrio libxio.so.1.0.1

vwrio: main.c io.c work.c signal.c common.c
	$(CC) $^ $(LIBS) $(INCLUDE) $(OPTS) -o $@ -Wall
	strip $@

d.vwrio: main.c io.c work.c signal.c common.c
	$(CC) $^ $(LIBS) $(INCLUDE) $(OPTS) $(DFLAGS) -g -o $@ -Wall

libxio.so: io.c work.c signal.c common.c
	$(CC) -fPIC -g -c -Wall $^
	$(CC) -shared -Wl,-soname,libxio.so -o libxio.so io.o work.o signal.o common.o
clean: 
	rm -f vwrio d.vwrio *.o *.so
