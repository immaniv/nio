CC=gcc

mem : mem.c common.c
	$(CC) -o $@ $^ -lrt

dmem : mem.c common.c
	$(CC) -g -o $@ $^ -lrt -DDEBUG -DDEBUGV
