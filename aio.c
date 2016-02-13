#include <sys/syscall.h>
#include <linux/aio_abi.h>
#include <sys/types.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <err.h>
#include <string.h>

static long io_setup(unsigned nr_reqs, aio_context_t *ctx) {
	return syscall(__NR_io_setup, nr_reqs, ctx);
}

static long io_submit(aio_context_t ctx, long n, struct iocb **paiocb) {
	return syscall(__NR_io_submit, ctx, n, paiocb);
}

static long io_getevents(aio_context_t ctx, long min_nr, long nr,
		struct io_event *events, struct timespec *tmo) {
	return syscall(__NR_io_getevents, ctx, min_nr, nr, events, tmo);
}

#if 0
static long io_cancel(aio_context_t ctx, struct iocb *aiocb,
		struct io_event *res) {
	return syscall(__NR_io_cancel, ctx, aiocb, res);
}

static long io_destroy(aio_context_t ctx) {
	return syscall(__NR_io_destroy, ctx);
}
#endif

aio_context_t aio_ctx;
struct iocb aio_cb;
struct iocb *aio_cbp = &aio_cb;
struct io_event aio_ev;

static ssize_t aio_pread(int fd, void *buf, size_t nbytes, off_t offset)
{
	aio_cb.aio_lio_opcode = IOCB_CMD_PREAD;
	aio_cb.aio_fildes = fd;
	aio_cb.aio_buf = (unsigned long) buf;
	aio_cb.aio_nbytes = nbytes;
	aio_cb.aio_offset = offset;

	if (io_submit(aio_ctx, 1, &aio_cbp) != 1)
		err(1, "aio submit failed");

	if (io_getevents(aio_ctx, 1, 1, &aio_ev, NULL) != 1)
		err(1, "aio getevents failed");

	if (aio_ev.res < 0) {
		errno = -aio_ev.res;
		return -1;
	}

	return aio_ev.res;
}

static ssize_t aio_pwrite(int fd, void *buf, size_t nbytes, off_t offset)
{
	aio_cb.aio_lio_opcode = IOCB_CMD_PWRITE;
	aio_cb.aio_fildes = fd;
	aio_cb.aio_buf = (unsigned long) buf;
	aio_cb.aio_nbytes = nbytes;
	aio_cb.aio_offset = offset;

	if (io_submit(aio_ctx, 1, &aio_cbp) != 1)
		err(1, "aio submit failed");

	if (io_getevents(aio_ctx, 1, 1, &aio_ev, NULL) != 1)
		err(1, "aio getevents failed");

	if (aio_ev.res < 0) {
		errno = -aio_ev.res;
		return -1;
	}

/*	if (!cached && fdatasync(fd) < 0)
		return -1;
*/
	return aio_ev.res;

#if 0
	aio_cb.aio_lio_opcode = IOCB_CMD_FDSYNC;
	if (io_submit(aio_ctx, 1, &aio_cbp) != 1)
		err(1, "aio fdsync submit failed");

	if (io_getevents(aio_ctx, 1, 1, &aio_ev, NULL) != 1)
		err(1, "aio getevents failed");

	if (aio_ev.res < 0)
		return aio_ev.res;
#endif
}

static void aio_setup(void)
{
	memset(&aio_ctx, 0, sizeof aio_ctx);
	memset(&aio_cb, 0, sizeof aio_cb);

	if (io_setup(1, &aio_ctx))
		err(2, "aio setup failed");

	/* make_request = write_test ? aio_pwrite : aio_pread; */
}


