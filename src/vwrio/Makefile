LIBS=-lrt -lpthread
INCLUDE=-I .
CC=gcc 
OPTS=-O3
DFLAGS=-DDEBUG

all: vwrio d.vwrio

vwrio: main.c io.c work.c common.c
	$(CC) $^ $(LIBS) $(INCLUDE) $(OPTS) -o $@
	strip $@

d.vwrio: main.c io.c work.c common.c
	$(CC) $^ $(LIBS) $(INCLUDE) $(OPTS) $(DFLAGS) -g -o $@

clean: 
	rm -f vwrio d.vwrio
