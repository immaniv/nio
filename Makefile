CC=gcc

mem : mem.c
	$(CC) -o $@ $^ -lrt
