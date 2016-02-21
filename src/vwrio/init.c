#include "common.h"

int
init_defaults (struct dev_opts *dopts) 
{
	dopts->bs = DEFAULT_BLOCK_SIZE;
	dopts->size = DEFAULT_DEV_SIZE;
	dopts->iter = DEFAULT_ITER;
	dopts->mode = N_READ;
	dopts->nthreads = DEFAULT_NTHREADS;
	dopts->type = SEQUENTIAL;
	dopts->read_ratio = DEFAULT_READ_RATIO;
	dopts->seq_ratio = DEFAULT_SEQ_RATIO;
	dopts->devselect = 0;
	dopts->indefinite = 0;
	dopts->verbose = 0;

	return 0;
}
