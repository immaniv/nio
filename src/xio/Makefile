CC=gcc 
AR=ar
LIBS=-lrt -lpthread
INCLUDE=-I .
DFLAGS=-DLINUX
DBGFLAGS=-DDEBUG -g
SRCS=main.c iothread.c work.c signal.c common.c init.c sysconf.c thread.c
CFLAGS=-fPIC -O3 -Wall $(DFLAGS) 
OBJS=$(SRCS:.c=.o) 
BIN=xio
DBIN=d.xio
SLIB=libxio.a
DLIB=libxio.so

all: $(SRCS) $(DLIB) $(SLIB) $(DBIN) $(BIN) 


$(DBIN): $(SRCS)
	$(CC) $(INCLUDE) $(DFLAGS) $(DBGFLAGS) $(CFLAGS) $^ -o $@ $(LIBS)

$(SLIB): $(OBJS) 
	$(AR) rcs $@ $^

$(DLIB): $(OBJS)
	$(CC) -shared -Wl,-soname,$@ -o $@ $^ $(LIBS)

$(BIN): $(SRCS)
	$(CC) $(INCLUDE) $(DFLAGS) $(CFLAGS) $^ -o $@ $(LIBS)
	strip $@

clean: 
	rm -f $(BIN) $(DBIN) $(SLIB) $(DLIB) $(OBJS)
