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
	int read_workers, write_workers, r_work, w_work;
	int seq_threads, rnd_threads, seq_t, rnd_t; 

	read_workers = 0;
	write_workers = 0;
	seq_threads = 0;
	rnd_threads = 0;
	
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

	dbg_printf(1,"Device: %s\n", iodev.devpath);

	if (iodev.type = MIXED) {
		seq_threads = (int) ((iodev.nthreads * iodev.seq_ratio)/100);
		rnd_threads = (int) iodev.nthreads - seq_threads;	
	}
	
	if (iodev.mode == MIXED) {
		read_workers = (int) ((iodev.nthreads * iodev.read_ratio)/100);
		write_workers = (int) iodev.nthreads - read_workers;
	}

	dbg_printf(1, "IO MODE: %c, R WORKERS: %d, W WORKERS: %d\n", GET_IO_MODE(iodev.mode), read_workers, write_workers);
	dbg_printf(1, "IO TYPE: %c, S THREADS: %d, R THREADS: %d\n", GET_IO_TYPE(iodev.type), seq_threads, rnd_threads);
	

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

	/* FIX THIS, YOU GIT */
	
	 
  	for (tnum = 0; tnum < iodev.nthreads; tnum++)  {
 		topts[tnum].thread_id = tnum;
		topts[tnum].opts = &iodev;
		
		if (iodev.type == MIXED) {
			for (seq_t = 0; seq_t < seq_threads; seq_t++) {	
				topts[tnum].t_type = SEQUENTIAL;
				if (iodev.mode == MIXED) {
					for (r_work = 0; r_work < (read_workers / seq_threads); r_work++)
						topts[tnum].t_mode = N_READ;
					for (w_work = 0; w_work < (write_workers / seq_threads); w_work++)
						topts[tnum].t_mode = N_WRITE;
				} else {
					topts[tnum].t_mode = iodev.mode;
				}
			} 
			
			for (rnd_t = 0; rnd_t < rnd_threads; rnd_t++) {	
				topts[tnum].t_type = RANDOM;
				if (iodev.mode == MIXED) {
					for (r_work = 0; r_work < (read_workers / seq_threads); r_work++)
						topts[tnum].t_mode = N_READ;
					for (w_work = 0; w_work < (write_workers / seq_threads); w_work++)
						topts[tnum].t_mode = N_WRITE;
				} else {
					topts[tnum].t_mode = iodev.mode;
				}
			} 
		} else {

		}

		dbg_printf(1, "Launching thread %d, mode = %c, type = %c\n", \
		tnum, \
		GET_IO_MODE(topts[tnum].t_mode), \
		GET_IO_TYPE(topts[tnum].t_type));
		
		/* static random seed */
		topts[tnum].rand_seed = (tnum + 1000); 
#ifndef DEBUG		
   		pthread_create(&numthreads[tnum], &attr, io_thread, (void *) &topts[tnum]); 
#endif 
   	}

#ifdef DEBUG
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
