CC=gcc 
AR=ar
LIBS=-lrt -lpthread
INCLUDE=-I .
CFLAGS=-fPIC -O3 -Wall -g
DFLAGS=-DLINUX
DEBUGFLAGS=-DDEBUG -g
SRCS=main.c iothread.c work.c signal.c common.c init.c sysconf.c thread.c
OBJS=$(SRCS:.c=.o) 
BIN=xio
DBIN=d.xio
SLIB=libxio.a
DLIB=libxio.so

all: $(SRCS) $(DLIB) $(SLIB) $(DBIN) $(BIN)


$(DBIN): $(OBJS)
	$(CC) $(LIBS) $(INCLUDE) $(DFLAGS) $(DEBUGFLAGS) $(CFLAGS) $^ -o $@ 

$(SLIB): $(OBJS) 
	$(AR) rcs $@ $^

$(DLIB): $(OBJS)
	$(CC) -shared -Wl,-soname,$@ -o $@ $^

$(BIN): $(OBJS)
	$(CC) $(LIBS) $(INCLUDE) $(DFLAGS) $(CFLAGS) $^ -o $@ 
	strip $@

clean: 
	rm -f $(BIN) $(DBIN) $(SLIB) $(DLIB) $(OBJS)
