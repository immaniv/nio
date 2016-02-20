#include <pthread.h>

#define N_READ   0
#define N_WRITE  1

#define DEFAULT_BLOCK_SIZE 	65536
#define DEFAULT_DEV_SIZE	16
#define DEFAULT_IO_MODE		N_READ
#define DEFAULT_ITER		16
#define DEFAULT_NTHREADS	8
#define DEFAULT_READ_RATIO	100
#define DEFAULT_SEQ_RATIO	100

#define SEQUENTIAL		0
#define	RANDOM			1
#define	MIXED			2

#define SECS	1
#define	MSECS	2
#define USECS	3
#define NSECS	4

#define GET_IO_MODE(a)  ((a) ? ((a == N_WRITE) ? 'W' : 'M') : 'R')
#define GET_IO_TYPE(a)  ((a) ? ((a == RANDOM) ? 'R' : 'M') : 'S') 

struct dev_opts {
	int fd;
	char *devpath;
	int bs;
	int size;
	int mode;
	int iter;
	int nthreads;
	char *buf;
	int devselect;
	int indefinite;
	int type;
	int read_ratio;
	int seq_ratio;
	int total_extents;
	int verbose;
};

struct worker_opts {
	int seq_threads;
	int rnd_threads;
	int rd_workers;
	int wr_workers;
	int seq_rd;
	int seq_wr;
	int rnd_rd;
	int rnd_wr;
}; 

struct thread_opts {
	struct dev_opts *opts;
	int thread_id;
	int t_type;
	int t_mode;
	int rand_seed;
	pthread_t tid;
};

void sigint_handler();
void sigkill_handler();
void sigterm_handler();

void usage(void);
void parse_args(int argc, char **argv, struct dev_opts *opts);
void dbg_printf(int verbosity, const char *format, ...);

void *io_thread (void *arg);
int io_cmd (int device_fd, char *io_buf, int bufsize, int type);
int worker_setup (struct worker_opts *io_workers, struct dev_opts *iodev);
int worker_alloc (struct worker_opts *io_workers, struct thread_opts *worker_thread);
void cleanup(pthread_mutex_t *mutex, struct dev_opts *opts);

double tdiff(struct timespec t1, struct timespec t2, short type);


