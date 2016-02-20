#define _XOPEN_SOURCE 600
#define _GNU_SOURCE 

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <linux/types.h>
#include <string.h>
#include <ctype.h>
#include <sys/statvfs.h>
#include <signal.h>
#include "common.h"

double IOPs, MBps; 
double min_lat, max_lat, avg_lat;
double r_IOPs, r_MBps, r_avg_lat;
double w_IOPs, w_MBps, w_avg_lat;
pthread_mutex_t mutexsum, rmutex, wmutex;
void *status;
pthread_mutex_t *wait_mutex;
pthread_attr_t attr;
pthread_t *numthreads;
int exit_status = 0;
struct dev_opts iodev;
struct thread_opts *topts;
int r_err, w_err;

int main (int argc, char *argv[])
{
	int i, tnum;
	struct dev_opts *iodevlist;
	struct worker_opts io_workers;

	memset(&io_workers, 0, sizeof(io_workers));
	
	iodev.bs = DEFAULT_BLOCK_SIZE;
	iodev.size = DEFAULT_DEV_SIZE;
	iodev.iter = DEFAULT_ITER;
	iodev.mode = N_READ;
	iodev.nthreads = DEFAULT_NTHREADS;
	iodev.type = SEQUENTIAL;
	iodev.read_ratio = DEFAULT_READ_RATIO;
	iodev.seq_ratio = DEFAULT_SEQ_RATIO;
	iodev.devselect = 0;
	iodev.indefinite = 0;
	iodev.verbose = 0;
	r_err = 0;
	w_err = 0;

	parse_args(argc, argv, &iodev);

	dbg_printf(1, "\
-----------------------------\n\
 Execution parameters: \n\
-----------------------------\n\
 Device: %s\n\
 Size: %d\n\
 Iterations: %d\n\
 Threads: %d\n\
 Mode: %d\n\
 Type: %d\n\
 Read Ratio: %d\n\
 Sequential Ratio: %d\n\
-----------------------------\n",
		iodev.devpath,
		iodev.size,
		iodev.iter,
		iodev.nthreads,
		iodev.mode,
		iodev.type,
		iodev.read_ratio,
		iodev.seq_ratio);

	worker_setup(&io_workers, &iodev);

	dbg_printf(1, "io mode: %c, R workers: %d, W workers: %d\n", GET_IO_MODE(iodev.mode), io_workers.rd_workers, io_workers.wr_workers);
	dbg_printf(1, "io type: %c, S threads: %d, R threads: %d\n", GET_IO_TYPE(iodev.type), io_workers.seq_threads, io_workers.rnd_threads);
	dbg_printf(1, "** spawn %d sequential io type / read io mode workers\n", io_workers.seq_rd);
	dbg_printf(1, "** spawn %d sequential io type / write io mode workers\n", io_workers.seq_wr);
	dbg_printf(1, "** spawn %d random io type / read io mode workers\n", io_workers.rnd_rd);
	dbg_printf(1, "** spawn %d random io type / write io mode workers\n", io_workers.rnd_wr);
	

	topts = (struct thread_opts *) malloc (sizeof(struct thread_opts) * iodev.nthreads);
	
	if ((iodev.fd = open(iodev.devpath, O_RDWR|O_DIRECT)) < 0) {
		fprintf(stdout, "Unable to open device: %s\n", iodev.devpath);
		perror("open");
		exit(1);
	}

	
	if (((iodev.buf = mmap(NULL, iodev.bs, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE, -1, 0)) == MAP_FAILED) \
		|| (mlock(iodev.buf, iodev.bs) == -1)) {
		perror("mmap");
		perror("mlock");
		exit(1);
	}

	memset(iodev.buf, 0, iodev.bs);

	signal(SIGINT, sigint_handler);
	signal(SIGTERM, sigterm_handler);
	signal(SIGKILL, sigkill_handler);

	numthreads = (pthread_t *) malloc(sizeof(pthread_t) * iodev.nthreads);
	wait_mutex = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t) * iodev.nthreads);
	
	pthread_mutex_init(&mutexsum, NULL);
	pthread_mutex_init(&rmutex, NULL);
	pthread_mutex_init(&wmutex, NULL);
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

  	for (tnum = 0; tnum < iodev.nthreads; tnum++)  {
 		topts[tnum].thread_id = tnum;
		topts[tnum].opts = &iodev;
	
		worker_alloc(&io_workers, &topts[tnum]);
	
		dbg_printf(1, "Launching worker_thread_id: %d, mode = %c, type = %c\n", \
			tnum, \
			GET_IO_MODE(topts[tnum].t_mode), \
			GET_IO_TYPE(topts[tnum].t_type));
		
		/* static random seed */
		topts[tnum].rand_seed = (tnum + 1000); 
#ifndef DEBUG		
   		pthread_create(&numthreads[tnum], &attr, io_thread, (void *) &topts[tnum]); 
	}
#else
   	}
	
	return 1;
#endif

	pthread_attr_destroy(&attr);

	for(i = 0; i < iodev.nthreads; i++) {
		pthread_join(numthreads[i], &status);
  	}

	fprintf(stdout, "dev: %s | n_threads: %02d | mode: %c | type: %c | blksize: %d (B) | iops: %.02f | MB/s: %.02f | svc_time: %.02f (ms)\n",
		iodev.devpath, 
		iodev.nthreads, 
		(iodev.mode) ? ((iodev.mode == N_WRITE) ? 'W' : 'M'): 'R', 
		(iodev.type) ? ((iodev.type == RANDOM) ? 'R' : 'M') : 'S',	
		iodev.bs, 
		IOPs,
		MBps,
		(avg_lat/1000.0)/iodev.nthreads);

	if (iodev.verbose) 	
		fprintf(stdout, "[READ] IO/s: %.02f, MB/s %.02f, Errors: %d\n[WRITE] IO/s: %.02f, MB/s %.02f, Errors: %d\n",
			r_IOPs, r_MBps, r_err, 
			w_IOPs, w_MBps, w_err);	

	cleanup(&mutexsum, &iodev);
	cleanup(&wmutex, NULL);
	cleanup(&rmutex, NULL); 
	pthread_exit(NULL);

	return 0;
}   
