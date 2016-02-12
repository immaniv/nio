#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct ll {
	struct ll *prev;
	struct ll *next;
	int n;
} llist;
	

int main(int argc, char **argv) 
{
	int i, c, r;
	llist *head, *curr, *next, *tail;

	if(argc < 2) {
		fprintf(stderr, "Usage: %s {num_list_elements}\n", argv[0]);
		exit(-1);
	} else {
		c = atoi(argv[1]);
	}

	head = curr = next = tail = NULL;
	
	/* allocate the beginning of the list */
	head = (llist *) malloc(sizeof(llist));

	head->prev = NULL;
	head->next = NULL;
	head->n = 0;
	curr = head;

	for(i = 1; i < c; i++) {
		next = (llist *) malloc(sizeof(llist));
		next->n = i;
		curr->next = next;
		next->prev = curr;
		curr = next;
	}
	
	/* mark the end of the list */	
	tail = curr;
	
	/* rewind to the beginning of the list */ 
	curr = head;
	
	/*
	while(curr) {
		fprintf(stdout, "curr_idx: %d\n", curr->n);
		curr = curr->next;
	}
	*/

	/* forward to the end of the list */
	curr = tail;

	/* 
	while(curr) {
		fprintf(stdout, "curr_idx: %d\n", curr->n);
		curr = curr->prev;
	}
	*/

	/* connect the tail with the head */

	tail->next = head;
	head->prev = tail;
	
	/* we now have a circular double linked list */	

	/* rewind again to the original beginning of the list */
	curr = head;

get_random:
	r = (random() % c);		
	
	while (curr) {
		if (curr->n == r) {
			fprintf(stdout, "curr_idx: %d\n", curr->n);
			goto get_random;
		} else {
			curr = curr->next;
		}
	}
	
	return 0;
}
