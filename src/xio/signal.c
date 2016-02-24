#include <signal.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "common.h"

void 
sigint_handler()
{
	sigterm_handler();
}

void 
sigterm_handler()
{
	int n;
	extern struct dev_opts iodev;
	extern pthread_mutex_t mutexsum;
	extern struct thread_opts *topts;
	fprintf(stdout, "Abort received. Cleaning up\n");
	for (n = 0; n < iodev.nthreads; n++) {
		fprintf(stdout, "Terminating thread: %d .. \n", n);
		pthread_cancel(topts[n].tid);
	}
	cleanup(&mutexsum, &iodev);
	exit(2);
}

void sigkill_handler()
{
	sigterm_handler();
}
