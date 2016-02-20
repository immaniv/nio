#include "common.h"
 
int
worker_setup(struct worker_opts *io_workers, struct dev_opts *iodev)
{
 	if (iodev->type == MIXED && iodev->seq_ratio !=0) {
                dbg_printf(1, "TYPE: %c, SEQ_RATIO: %d, RAND_RATIO: %d\n", GET_IO_TYPE(iodev->type), iodev->seq_ratio, (100 - iodev->seq_ratio));
                io_workers->seq_threads = (int) ((iodev->nthreads * iodev->seq_ratio)/100);
                if (!io_workers->seq_threads)
                        io_workers->seq_threads = 1;
                io_workers->rnd_threads = (int) iodev->nthreads - io_workers->seq_threads;
        } else {
                dbg_printf(1, "[ERROR] I/O Type ratio mismatch. MODE: %c, READ_RATIO: %d\n", GET_IO_TYPE(iodev->type), iodev->seq_ratio);
                return -1;
        }


        if (iodev->mode == MIXED && iodev->read_ratio != 0) {
                dbg_printf(1, "MODE: %c, READ_RATIO: %d, WRITE_RATIO: %d\n", GET_IO_MODE(iodev->mode), iodev->read_ratio, (100 - iodev->read_ratio));
                io_workers->rd_workers = (int) ((iodev->nthreads * iodev->read_ratio)/100);
                if (!io_workers->rd_workers)
                        io_workers->rd_workers = 1;
                io_workers->wr_workers = (int) iodev->nthreads - io_workers->rd_workers;
        } else {
                dbg_printf(1, "[ERROR] I/O Mode ratio mismatch. MODE: %c, READ_RATIO: %d\n", GET_IO_MODE(iodev->mode), iodev->read_ratio);
                return -1;
        }
	
        if ((iodev->type == MIXED) && (iodev->mode == MIXED)) {
		if (io_workers->seq_threads > io_workers->rnd_threads && io_workers->rd_workers > io_workers->wr_workers) {
			io_workers->seq_rd = (int) ((io_workers->seq_threads + io_workers->rd_workers) / 2);
			io_workers->seq_wr = io_workers->seq_threads - io_workers->seq_rd;
			io_workers->rnd_rd = (int) ((io_workers->rnd_threads + io_workers->wr_workers) / 2);
			io_workers->rnd_wr = io_workers->rnd_threads - io_workers->rnd_rd;
		} else if (io_workers->seq_threads > io_workers->rnd_threads && io_workers->wr_workers > io_workers->rd_workers) {
			io_workers->seq_wr = (int) ((io_workers->seq_threads + io_workers->wr_workers) / 2);
			io_workers->seq_rd = io_workers->seq_threads - io_workers->seq_wr;
			io_workers->rnd_wr = (int) ((io_workers->rnd_threads + io_workers->rd_workers) / 2);
			io_workers->rnd_rd = io_workers->rnd_threads - io_workers->rnd_wr;
		} else if (io_workers->rnd_threads > io_workers->seq_threads && io_workers->rd_workers > io_workers->wr_workers) {
			io_workers->rnd_rd = (int) ((io_workers->rnd_threads + io_workers->rd_workers) / 2);
			io_workers->rnd_wr = io_workers->rnd_threads - io_workers->rnd_rd;
			io_workers->seq_rd = (int) ((io_workers->seq_threads + io_workers->wr_workers) / 2);
			io_workers->seq_wr = io_workers->seq_threads - io_workers->seq_rd;
		} else if (io_workers->rnd_threads > io_workers->seq_threads && io_workers->wr_workers > io_workers->rd_workers) {
			io_workers->rnd_wr = (int) ((io_workers->rnd_threads + io_workers->wr_workers) / 2);
			io_workers->rnd_rd = io_workers->rnd_threads - io_workers->rnd_wr;
			io_workers->seq_wr = (int) ((io_workers->seq_threads + io_workers->rd_workers) / 2);
			io_workers->seq_rd = io_workers->seq_threads - io_workers->seq_wr;
		} else if (io_workers->rnd_threads == io_workers->seq_threads && io_workers->wr_workers == io_workers->rd_workers) {
			io_workers->rnd_rd = io_workers->seq_rd = io_workers->seq_wr = io_workers->rnd_wr = (int) iodev->nthreads / 4;
		}
	}
	
	return 0;
}

int worker_alloc(struct worker_opts *io_workers, struct thread_opts *worker_thread)
{
	if (worker_thread->opts->type == MIXED && worker_thread->opts->mode == MIXED) {
		if (io_workers->seq_rd) {
			worker_thread->t_type = SEQUENTIAL;
			worker_thread->t_mode = N_READ;
			io_workers->seq_rd--;
		} else if (io_workers->seq_wr) {
			worker_thread->t_type = SEQUENTIAL;
			worker_thread->t_mode = N_WRITE;
			io_workers->seq_wr--;
		} else if (io_workers->rnd_rd) {
			worker_thread->t_type = RANDOM;
			worker_thread->t_mode = N_READ;
			io_workers->rnd_rd--;
		} else if (io_workers->rnd_wr) {
			worker_thread->t_type = RANDOM;
			worker_thread->t_mode = N_WRITE;
			io_workers->rnd_wr--;
		}
	} else {
		worker_thread->t_mode = worker_thread->opts->mode;

	}
	dbg_printf(1, "Allocating worker_thread_id: %d, io type: %c, io mode: %c\n",\
		worker_thread->thread_id,\
		GET_IO_TYPE(worker_thread->t_type),\
		GET_IO_MODE(worker_thread->t_mode));

	return 0;

}
