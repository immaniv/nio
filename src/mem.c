#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include "common.h"

#define NELEMENTS 8
#define ESIZE 32

typedef struct ll {
	struct ll *prev;
	struct ll *next;
	int idx;
	char n[ESIZE];
} llist;

struct timespec start, end;

int 
main(int argc, char **argv) 
{
	int i, c, r; 
	char t[NELEMENTS];
	llist *head, *curr, *next, *tail;

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
#ifdef DEBUG
	fprintf(stdout, "Allocated idx: %d\n", i);
#endif 
	}
	clock_gettime(CLOCK_MONOTONIC, &end);	

#ifdef DEBUG
	fprintf(stdout, "Allocation time: %ld usecs\n", tdiff(start, end));
#endif 
		
	/* mark the end of the list */	
	tail = curr;
	
	/* connect the tail with the head to create a circular linked list*/

	tail->next = head;
	head->prev = tail;

	/* rewind to the original beginning of the list */
	curr = head;

	clock_gettime(CLOCK_MONOTONIC, &start);
	while (curr->idx) {
#ifdef DEBUGV
		fprintf(stdout, "Current link index: %d\n", curr->idx);
#endif
		if (curr->idx == (NELEMENTS - 1)) 
			break;
		
		curr = curr->next;
		
	}
	clock_gettime(CLOCK_MONOTONIC, &end);

#ifdef DEBUG
	fprintf(stdout, "Traversal time: %ld usecs\n", tdiff(start, end));
#endif 
	return 0;
}
