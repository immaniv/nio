#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/types.h>
#include <string.h>
#include <ctype.h>
#include <sys/statvfs.h>
#include <signal.h>
#include <stdarg.h>

#include "common.h"


void 
dbg_printf(int verbosity, const char *format, ...) 
{
#ifdef DEBUG
	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
#endif
}

void 
cleanup(pthread_mutex_t *mutex, struct dev_opts *opts)
{
	pthread_mutex_destroy(mutex);
	if (opts != NULL) {
        	close(opts->fd); 
		munmap(opts->buf, opts->blksize);
	}
}

void 
usage(void) 
{
	extern char *__progname;
	
	fprintf(stdout, "\nUsage: %s [options]\n%s%s%s%s", __progname, "\
	\n\
 -d $device      - /dev/sdX (block device)\n\
 -b $blksize     - in bytes (default is 65536 bytes - min is 512 bytes, max is 4194304 bytes)\n\
 -s $devsize     - in MB (default is 16 MB - min 4 MB)\n\
 -n $threads     - number of IO threads (default is 8 - min is 1, max is 32)\n\
 -i $iterations  - number of times to run (default is 16 - min is 16)\n\
 -t $type        - S for SEQUENTIAL (default), R for RANDOM, M for MIXED\n\
 -T $type_ratio  - ratio of SEQUENTIAL type IO to RANDOM type IO (default is 100\% SEQUENTIAL) \n\
 -m $mode        - R for READ (default), W for WRITE, M for MIXED\n\
 -M $mode_ratio  - ratio of READ mode IO to WRITE mode IO (default is 100\% READ)\n\
 -D 		 - use Linux DIRECT IO interface\n\
 -C	         - use *all* CPUs to drive I/O (one cpu per worker thread). CPUs will be \n\
                   oversubscribed in a round-robin manner if the threads specified exceed\n\
                   the number of available CPUs. By default, all the worker threads are\n\
                   scheduled by the OS.\n\
 -c              - specify CPU binding on a per thread basis. Ex: c0:t1,c1,t2 will bind\n\
                   thread t1 to CPU 0 and thread t2 to CPU 1. Currently, thread naming is\n\
                   arbitrary. For instance, if you specify '-n4' (4 worker threads), the \n\
                   first worker thread would be t1, the second t2 and so on. \n\
 -I              - run indefinitely and report periodic stats\n\
 -v              - verbose output\n\
 -h                this help\n\n",\
"Example:\n",\
__progname, " -d /dev/sda -b 4096 -s 64 -n 8 -i 64\n\n"\
	);
}


void 
parse_args (int argc, char **argv, struct dev_opts *opts)
{
	int carg;

	if (argc < 2) {
		usage();
		exit(1);
	}

	opterr = 0;
	
	while ((carg = getopt(argc, argv, "hIDCvd:m:c:b:t:s:n:i:M:T:")) != -1)
		switch (carg) {
			case 'M':
				opts->read_ratio = atoi(optarg);
				if (opts->read_ratio > 100 || opts->read_ratio < 0) {
					usage();
					exit(1);
				}
				break;
			case 'D':
				opts->use_dio = 1;
				break;
			case 'C':
				opts->use_rr_cpu = 1;
				break;
			case 'T':
				opts->seq_ratio = atoi(optarg);
				if (opts->seq_ratio > 100 || opts->seq_ratio < 0) {
					usage();
					exit(1);
				}
				break;
			case 'd':
				opts->devpath = optarg;
				opts->devselect = 1;
				break;
			case 'b':
				opts->blksize = atoi(optarg);
				if (opts->blksize < 512 || opts->blksize > (1024*4096)) {
					usage();
					exit(1);
				}
				break;
			case 's':
				opts->size = atoi(optarg);
				if (opts->size < 4) {
					usage();
					exit(1);
				}
				break;
			case 'n':
				opts->nthreads = atoi(optarg);
				if (opts->nthreads > 32 || opts->nthreads < 1) {
					usage();
					exit(1);
				}
				break;	
			case 'i':
				opts->iter = atoi(optarg);
				if (opts->iter < 16) {
					usage();
					exit(1);
				}
				break;
			case 'I':
				opts->indefinite = 1;
				break;
			case 'm':
				if (optarg[0] == 'R') {
					opts->mode = N_READ;
				} else if (optarg[0] == 'W') {
					opts->mode = N_WRITE;
					opts->read_ratio = 0;
				} else if (optarg[0] == 'M') {
					opts->mode = MIXED;
				} else {
					usage();
					exit(1);
				}
				break;
			case 't':
				if (optarg[0] == 'S' || optarg[0] == 's') 
					opts->type = SEQUENTIAL;
				else if (optarg[0] == 'R' || optarg[0] == 'r') {
					opts->type = RANDOM;
					opts->seq_ratio = 0;
				} else if (optarg[0] == 'M' || optarg[0] == 'm')
					opts->type = MIXED;
				else {
					usage();
					exit(1);
				}
				break;
			case 'h':
				usage();
				exit(1);
			case 'v':
				opts->verbose = 1;
				break;
			default:
				usage();
				exit(1);
		}

	if (!opts->devselect) {
		usage();
		exit(1);
	}
	
	dbg_printf(1, "\
-----------------------------\n\
 Execution parameters: \n\
-----------------------------\n\
 Device: %s\n\
 Size: %d\n\
 Iterations: %d\n\
 Threads: %d\n\
 Mode: %c\n\
 Type: %c\n\
 Read Ratio: %d\n\
 Sequential Ratio: %d\n\
 Direct IO Interface: %s\n\
-----------------------------\n",
		opts->devpath,
		opts->size,
		opts->iter,
		opts->nthreads,
		GET_IO_MODE(opts->mode),
		GET_IO_TYPE(opts->type),
		opts->read_ratio,
		opts->seq_ratio,
		(opts->use_dio) ? "Specified" : "Not Specified");
}
