#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sched.h>
#include <pthread.h>

#include "common.h"
#include "debug.h"

#define _GNU_SOURCE

#define NELEMENTS 8
#define ESIZE 32

typedef struct ll {
	struct ll *prev;
	struct ll *next;
	int idx;
	char n[ESIZE];
} llist;

void *lwalk(void *threadid) {
	long tid;
	tid = (long) threadid;
	
	pthread_exit(NULL);
}

struct timespec start, end;

int 
main(int argc, char **argv) 
{
	int i, c, r; 
	int nprocs;
	cpu_set_t cpuset;


	char t[NELEMENTS];
	llist *head, *curr, *next, *tail;

	nprocs = sysconf(_SC_NPROCESSORS_ONLN);

	head = curr = next = tail = NULL;
	memset((char *) t, 0, NELEMENTS);
	
	clock_gettime(CLOCK_MONOTONIC, &start);	
	head = (llist *) malloc(sizeof(llist));

	head->prev = NULL;
	head->next = NULL;
	memset((char *) head->n, (random() % 255), ESIZE);
	head->idx = 0;
	curr = head;

	for(i = 1; i < NELEMENTS; i++) {
		next = (llist *) malloc(sizeof(llist));
		memset((char *) next->n, (random() % 255), ESIZE);
		next->idx = i;
		curr->next = next;
		next->prev = curr;
		curr = next;
		
		dbg_printf(1, "Allocated idx: %d\n", i);
	}
	clock_gettime(CLOCK_MONOTONIC, &end);	

	dbg_printf(1, "Allocation time: %ld usecs\n", tdiff(start, end));
		
	/* mark the end of the list */	
	tail = curr;
	
	/* connect the tail with the head to create a circular linked list*/

	tail->next = head;
	head->prev = tail;

	/* rewind to the original beginning of the list */
	curr = head;

	clock_gettime(CLOCK_MONOTONIC, &start);
	while (curr->idx) {
	
		dbg_printf(1, "Current link index: %d\n", curr->idx);
		
		if (curr->idx == (NELEMENTS - 1)) 
			break;
		
		curr = curr->next;
		
	}
	clock_gettime(CLOCK_MONOTONIC, &end);

	dbg_printf(1, "Traversal time: %ld usecs\n", tdiff(start, end));

	return 0;
}
