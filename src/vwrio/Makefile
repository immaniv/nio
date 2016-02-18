LIBS=-lrt -lpthread
INCLUDE=-I .
CC=gcc
OPTS=-O3

all: vwrio d.vwrio

vwrio: main.c common.c io.c
	$(CC) -o $@ $^ $(LIBS) $(INCLUDE) $(OPTS)
	strip $@

d.vwrio: main.c common.c io.c
	cc -g -o d.vwrio main.c common.c io.c -lrt -lpthread -I . -DDEBUG

clean: 
	rm -f vwrio d.vwrio
