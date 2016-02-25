#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>

#include "common.h"
#include "timing.h"

void *io_thread (void *arg)
{
	
	extern pthread_mutex_t mutexsum, rmutex, wmutex;
	extern double IOPs, MBps;
	extern double r_IOPs, r_MBps, r_avg_lat, r_err;
	extern double w_IOPs, w_MBps, w_avg_lat, w_err;
	/* extern double min_lat, max_lat, avg_lat; */
	extern double avg_lat; 

	struct dev_opts *n_iodev;
	struct timespec startt, endt, iostartt, ioendt;
	int n, iter, nbytes, id, myfd;
	double diff, iodiff;
	double lIOps, lMBps, lavg_lat;
	struct thread_opts *myopts;
	int total_extent;
	int lr_err, lw_err;

	myopts = (struct thread_opts *) arg;
	
	n_iodev = (struct dev_opts *) myopts->opts;
	id = (int) myopts->thread_id;
	
	pthread_t myid;
	myid = pthread_self();
	myopts->tid = myid;

	srandom(myopts->rand_seed);

	myfd = dup(n_iodev->fd);

	total_extent = ((n_iodev->size * 1024 * 1024)/n_iodev->blksize);

	dbg_printf(1, "Running worker_thread_d %d in mode %c, type %c\n", \
		id, GET_IO_MODE(myopts->t_mode), GET_IO_TYPE(myopts->t_type));

	
RUN_INDEFINITELY:
	lr_err = 0;
	lw_err = 0;	
	lIOps = 0.0;
	lMBps = 0.0;
	lavg_lat = 0.0;
	diff = 0.0;
	iodiff = 0.0;
	n = 0;
        iter = 0;
        nbytes = 0;

	if (myopts->t_type == SEQUENTIAL) {
		GET_SYS_CLOCK(&startt);
		for (iter = 0; iter < n_iodev->iter; iter++) {
			/* lseek(myfd, 0, SEEK_SET); */
			if (myopts->t_mode == N_READ) {
				for (n = 0; n < total_extent; n++) {
					GET_SYS_CLOCK(&iostartt);
					if ((nbytes = (pread(myfd, n_iodev->buf, n_iodev->blksize, (n * n_iodev->blksize)))) != n_iodev->blksize) {
						lr_err++;
						/* perror("read"); */
					}
					GET_SYS_CLOCK(&ioendt);
					iodiff += TDIFF(iostartt, ioendt)/USECS; 
				}
			} else if (myopts->t_mode == N_WRITE) {
				for (n = 0; n < total_extent; n++) {
					GET_SYS_CLOCK(&iostartt);
					if ((nbytes = (pwrite(myfd, n_iodev->buf, n_iodev->blksize, (n * n_iodev->blksize)))) != n_iodev->blksize) { 
						lw_err++;
						/* perror("write"); */
					}
					GET_SYS_CLOCK(&ioendt);
					iodiff += TDIFF(iostartt, ioendt)/USECS; 
				}
			}
			iodiff /= n;
		}
		GET_SYS_CLOCK(&endt);
	} else if (myopts->t_type == RANDOM) {
		GET_SYS_CLOCK(&startt);
		for (iter = 0; iter < n_iodev->iter; iter++) {
			lseek(myfd, 0, SEEK_SET);
			if (myopts->t_mode == N_READ) {
				for (n = 0; n < total_extent; n++) {
					/* lseek(myfd, (n_iodev->blksize * (rand() % total_extent)), SEEK_SET); */
					GET_SYS_CLOCK(&iostartt);
					if ((nbytes = (pread(myfd, n_iodev->buf, n_iodev->blksize, (n_iodev->blksize * (rand() % total_extent))))) != n_iodev->blksize) {
						lr_err++;
						/* perror("read"); */
					}
					GET_SYS_CLOCK(&ioendt);
					iodiff += TDIFF(iostartt, ioendt)/USECS; 
				}
			} else if (myopts->t_mode == N_WRITE) {
				for (n = 0; n < total_extent; n++) {
					/* lseek(myfd, (n_iodev->blksize * (rand() % total_extent)), SEEK_SET); */
					GET_SYS_CLOCK(&iostartt);
					if ((nbytes = (pwrite(myfd, n_iodev->buf, n_iodev->blksize, (n_iodev->blksize * (rand() % total_extent))))) != n_iodev->blksize) { 
						lw_err++;
						/* perror("write"); */
					}
					GET_SYS_CLOCK(&ioendt);
					iodiff += TDIFF(iostartt, ioendt)/USECS;
				}
			}
			iodiff /= n;
		}
		GET_SYS_CLOCK(&endt);
	}

	diff = GET_DIFF_SECS(TDIFF(startt, endt));
	
	lIOps = n/(diff/iter); 
	lMBps = ((n/(diff/iter))*n_iodev->blksize)/(1024*1024);
	lavg_lat = (iodiff/iter);
	
	if (n_iodev->verbose || n_iodev->indefinite) {
		fprintf(stdout, "thread id: %d | blksize: %d (B) | mode: %c | type: %c | iops: %.02f | MB/s: %.02f | svc_time: %.2f (ms)\n", \
			id, \
			n_iodev->blksize, \
			GET_IO_MODE(myopts->t_mode), \
			GET_IO_TYPE(myopts->t_type), \
			lIOps, \
			lMBps, \
			lavg_lat/1000);
	}
	
	if (n_iodev->indefinite) 
		goto RUN_INDEFINITELY;

	pthread_mutex_lock(&mutexsum);

	if (myopts->t_mode == N_READ) {
		pthread_mutex_lock(&rmutex);
		r_IOPs += lIOps;
		r_MBps += lMBps;
		r_avg_lat += lavg_lat;
		r_err += lr_err;
	} else  {
		pthread_mutex_lock(&wmutex);
		w_IOPs += lIOps;
		w_MBps += lMBps;
		w_avg_lat += lavg_lat;
		w_err += lw_err;
	}

	IOPs += lIOps;
	MBps += lMBps;
	avg_lat += lavg_lat;
	
	if (myopts->t_mode == N_READ) 
		pthread_mutex_unlock(&rmutex);
	else 
		pthread_mutex_unlock(&wmutex);

	pthread_mutex_unlock(&mutexsum);

	pthread_exit((void*) 0);	

}