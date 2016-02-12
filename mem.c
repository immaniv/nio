#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#define NELEMENTS (1024 * 1024)
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
	llist *head, *curr, *next, *tail;

	head = curr = next = tail = NULL;
	
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
	}
	clock_gettime(CLOCK_MONOTONIC, &end);	
	
	/* mark the end of the list */	
	tail = curr;
	
	/* connect the tail with the head */

	tail->next = head;
	head->prev = tail;
	
	/* we now have a circular double linked list */	

	/* rewind to the original beginning of the list */
	curr = head;

get_random:
	r = (random() % NELEMENTS);		
	
	while (curr) {
		if (curr->idx == r) {
			fprintf(stdout, "curr_idx: %08d, contents: %s\n", curr->idx, curr->n);
			goto get_random;
		} else {
			curr = curr->next;
		}
	}
	return 0;
}
