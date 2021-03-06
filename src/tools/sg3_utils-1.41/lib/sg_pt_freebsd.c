/*
 * Copyright (c) 2005-2015 Douglas Gilbert.
 * All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the BSD_LICENSE file.
 */

/* sg_pt_freebsd version 1.13 20150511 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>
#include <errno.h>
#include <err.h>
#include <camlib.h>
#include <cam/scsi/scsi_message.h>
// #include <sys/ata.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <glob.h>
#include <fcntl.h>
#include <stddef.h>

#include "sg_pt.h"
#include "sg_lib.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


#define FREEBSD_MAXDEV 64
#define FREEBSD_FDOFFSET 16;


struct freebsd_dev_channel {
  char* devname;                // the SCSI device name
  int   unitnum;                // the SCSI unit number
  struct cam_device* cam_dev;
};

// Private table of open devices: guaranteed zero on startup since
// part of static data.
static struct freebsd_dev_channel *devicetable[FREEBSD_MAXDEV];

#define DEF_TIMEOUT 60000       /* 60,000 milliseconds (60 seconds) */

struct sg_pt_freebsd_scsi {
    struct cam_device* cam_dev; // copy held for error processing
    union ccb *ccb;
    unsigned char * cdb;
    int cdb_len;
    unsigned char * sense;
    int sense_len;
    unsigned char * dxferp;
    int dxfer_len;
    int dxfer_dir;
    int scsi_status;
    int resid;
    int sense_resid;
    int in_err;
    int os_err;
    int transport_err;
};

struct sg_pt_base {
    struct sg_pt_freebsd_scsi impl;
};

#ifdef __GNUC__
static int pr2ws(const char * fmt, ...)
        __attribute__ ((format (printf, 1, 2)));
#else
static int pr2ws(const char * fmt, ...);
#endif


static int
pr2ws(const char * fmt, ...)
{
    va_list args;
    int n;

    va_start(args, fmt);
    n = vfprintf(sg_warnings_strm ? sg_warnings_strm : stderr, fmt, args);
    va_end(args);
    return n;
}


/* Returns >= 0 if successful. If error in Unix returns negated errno. */
int
scsi_pt_open_device(const char * device_name, int read_only, int verbose)
{
    int oflags = 0 /* O_NONBLOCK*/ ;

    oflags |= (read_only ? O_RDONLY : O_RDWR);
    return scsi_pt_open_flags(device_name, oflags, verbose);
}

/* Similar to scsi_pt_open_device() but takes Unix style open flags OR-ed
 * together. The 'flags' argument is ignored in FreeBSD.
 * Returns >= 0 if successful, otherwise returns negated errno. */
int
scsi_pt_open_flags(const char * device_name,
                   int flags __attribute__ ((unused)), int verbose)
{
    struct freebsd_dev_channel *fdchan;
    struct cam_device* cam_dev;
    int k;

    // Search table for a free entry
    for (k = 0; k < FREEBSD_MAXDEV; k++)
        if (! devicetable[k])
            break;

    // If no free entry found, return error.  We have max allowed number
    // of "file descriptors" already allocated.
    if (k == FREEBSD_MAXDEV) {
        if (verbose)
            pr2ws("too many open file descriptors (%d)\n", FREEBSD_MAXDEV);
        errno = EMFILE;
        return -1;
    }

    fdchan = (struct freebsd_dev_channel *)
                calloc(1,sizeof(struct freebsd_dev_channel));
    if (fdchan == NULL) {
        // errno already set by call to calloc()
        return -1;
    }

    if (! (fdchan->devname = (char *)calloc(1, DEV_IDLEN+1)))
         return -1;

    if (cam_get_device(device_name, fdchan->devname, DEV_IDLEN,
                       &(fdchan->unitnum)) == -1) {
        if (verbose)
            pr2ws("bad device name structure\n");
        errno = EINVAL;
        return -1;
    }

    if (! (cam_dev = cam_open_spec_device(fdchan->devname,
                                          fdchan->unitnum, O_RDWR, NULL))) {
        if (verbose)
            pr2ws("cam_open_spec_device: %s\n", cam_errbuf);
        errno = EPERM;    /* permissions or no CAM */
        return -1;
    }
    fdchan->cam_dev = cam_dev;
    // return pointer to "file descriptor" table entry, properly offset.
    devicetable[k] = fdchan;
    return k + FREEBSD_FDOFFSET;
}

/* Returns 0 if successful. If error in Unix returns negated errno. */
int
scsi_pt_close_device(int device_fd)
{
    struct freebsd_dev_channel *fdchan;
    int fd = device_fd - FREEBSD_FDOFFSET;

    if ((fd < 0) || (fd >= FREEBSD_MAXDEV)) {
        errno = ENODEV;
        return -1;
    }
    fdchan = devicetable[fd];
    if (NULL == fdchan) {
        errno = ENODEV;
        return -1;
    }
    if (fdchan->devname)
        free(fdchan->devname);
    if (fdchan->cam_dev)
        cam_close_device(fdchan->cam_dev);
    free(fdchan);
    devicetable[fd] = NULL;
    return 0;
}

struct sg_pt_base *
construct_scsi_pt_obj()
{
    struct sg_pt_freebsd_scsi * ptp;

    /* The following 2 lines are temporary. It is to avoid a NULL pointer
     * crash when an old utility is used with a newer library built after
     * the sg_warnings_strm cleanup */
    if (NULL == sg_warnings_strm)
        sg_warnings_strm = stderr;

    ptp = (struct sg_pt_freebsd_scsi *)
                calloc(1, sizeof(struct sg_pt_freebsd_scsi));
    if (ptp) {
        memset(ptp, 0, sizeof(struct sg_pt_freebsd_scsi));
        ptp->dxfer_dir = CAM_DIR_NONE;
    }
    return (struct sg_pt_base *)ptp;
}

void
destruct_scsi_pt_obj(struct sg_pt_base * vp)
{
    struct sg_pt_freebsd_scsi * ptp = &vp->impl;

    if (ptp) {
        if (ptp->ccb)
            cam_freeccb(ptp->ccb);
        free(ptp);
    }
}

void
clear_scsi_pt_obj(struct sg_pt_base * vp)
{
    struct sg_pt_freebsd_scsi * ptp = &vp->impl;

    if (ptp) {
        if (ptp->ccb)
            cam_freeccb(ptp->ccb);
        memset(ptp, 0, sizeof(struct sg_pt_freebsd_scsi));
        ptp->dxfer_dir = CAM_DIR_NONE;
    }
}

void
set_scsi_pt_cdb(struct sg_pt_base * vp, const unsigned char * cdb, int cdb_len)
{
    struct sg_pt_freebsd_scsi * ptp = &vp->impl;

    if (ptp->cdb)
        ++ptp->in_err;
    ptp->cdb = (unsigned char *)cdb;
    ptp->cdb_len = cdb_len;
}

void
set_scsi_pt_sense(struct sg_pt_base * vp, unsigned char * sense,
                  int max_sense_len)
{
    struct sg_pt_freebsd_scsi * ptp = &vp->impl;

    if (ptp->sense)
        ++ptp->in_err;
    memset(sense, 0, max_sense_len);
    ptp->sense = sense;
    ptp->sense_len = max_sense_len;
}

/* Setup for data transfer from device */
void
set_scsi_pt_data_in(struct sg_pt_base * vp, unsigned char * dxferp,
                    int dxfer_len)
{
    struct sg_pt_freebsd_scsi * ptp = &vp->impl;

    if (ptp->dxferp)
        ++ptp->in_err;
    if (dxfer_len > 0) {
        ptp->dxferp = dxferp;
        ptp->dxfer_len = dxfer_len;
        ptp->dxfer_dir = CAM_DIR_IN;
    }
}

/* Setup for data transfer toward device */
void
set_scsi_pt_data_out(struct sg_pt_base * vp, const unsigned char * dxferp,
                     int dxfer_len)
{
    struct sg_pt_freebsd_scsi * ptp = &vp->impl;

    if (ptp->dxferp)
        ++ptp->in_err;
    if (dxfer_len > 0) {
        ptp->dxferp = (unsigned char *)dxferp;
        ptp->dxfer_len = dxfer_len;
        ptp->dxfer_dir = CAM_DIR_OUT;
    }
}

void
set_scsi_pt_packet_id(struct sg_pt_base * vp __attribute__ ((unused)),
                      int pack_id __attribute__ ((unused)))
{
}

void
set_scsi_pt_tag(struct sg_pt_base * vp, uint64_t tag __attribute__ ((unused)))
{
    struct sg_pt_freebsd_scsi * ptp = &vp->impl;

    ++ptp->in_err;
}

void
set_scsi_pt_task_management(struct sg_pt_base * vp,
                            int tmf_code __attribute__ ((unused)))
{
    struct sg_pt_freebsd_scsi * ptp = &vp->impl;

    ++ptp->in_err;
}

void
set_scsi_pt_task_attr(struct sg_pt_base * vp,
                      int attrib __attribute__ ((unused)),
                      int priority __attribute__ ((unused)))
{
    struct sg_pt_freebsd_scsi * ptp = &vp->impl;

    ++ptp->in_err;
}

void
set_scsi_pt_flags(struct sg_pt_base * objp, int flags)
{
    if (objp) { ; }     /* unused, suppress warning */
    if (flags) { ; }     /* unused, suppress warning */
}

/* Executes SCSI command (or at least forwards it to lower layers).
 * Clears os_err field prior to active call (whose result may set it
 * again). */
int
do_scsi_pt(struct sg_pt_base * vp, int device_fd, int time_secs, int verbose)
{
    int fd = device_fd - FREEBSD_FDOFFSET;
    struct sg_pt_freebsd_scsi * ptp = &vp->impl;
    struct freebsd_dev_channel *fdchan;
    union ccb *ccb;
    int len, timout_ms;

    ptp->os_err = 0;
    if (ptp->in_err) {
        if (verbose)
            pr2ws("Replicated or unused set_scsi_pt...\n");
        return SCSI_PT_DO_BAD_PARAMS;
    }
    if (NULL == ptp->cdb) {
        if (verbose)
            pr2ws("No command (cdb) given\n");
        return SCSI_PT_DO_BAD_PARAMS;
    }

    if ((fd < 0) || (fd >= FREEBSD_MAXDEV)) {
        if (verbose)
            pr2ws("Bad file descriptor\n");
        ptp->os_err = ENODEV;
        return -ptp->os_err;
    }
    fdchan = devicetable[fd];
    if (NULL == fdchan) {
        if (verbose)
            pr2ws("File descriptor closed??\n");
        ptp->os_err = ENODEV;
        return -ptp->os_err;
    }
    if (NULL == fdchan->cam_dev) {
        if (verbose)
            pr2ws("No open CAM device\n");
        return SCSI_PT_DO_BAD_PARAMS;
    }

    if (NULL == ptp->ccb) {     /* re-use if we have one already */
        if (! (ccb = cam_getccb(fdchan->cam_dev))) {
            if (verbose)
                pr2ws("cam_getccb: failed\n");
            ptp->os_err = ENOMEM;
            return -ptp->os_err;
        }
        ptp->ccb = ccb;
    } else
        ccb = ptp->ccb;

    // clear out structure, except for header that was filled in for us
    bzero(&(&ccb->ccb_h)[1],
            sizeof(struct ccb_scsiio) - sizeof(struct ccb_hdr));

    timout_ms = (time_secs > 0) ? (time_secs * 1000) : DEF_TIMEOUT;
    cam_fill_csio(&ccb->csio,
                  /* retries */ 1,
                  /* cbfcnp */ NULL,
                  /* flags */ ptp->dxfer_dir,
                  /* tagaction */ MSG_SIMPLE_Q_TAG,
                  /* dataptr */ ptp->dxferp,
                  /* datalen */ ptp->dxfer_len,
                  /* senselen */ ptp->sense_len,
                  /* cdblen */ ptp->cdb_len,
                  /* timeout (millisecs) */ timout_ms);
    memcpy(ccb->csio.cdb_io.cdb_bytes, ptp->cdb, ptp->cdb_len);

    if (cam_send_ccb(fdchan->cam_dev, ccb) < 0) {
        if (verbose) {
            warn("error sending SCSI ccb");
 #if __FreeBSD_version > 500000
            cam_error_print(fdchan->cam_dev, ccb, CAM_ESF_ALL,
                            CAM_EPF_ALL, stderr);
 #endif
        }
        cam_freeccb(ptp->ccb);
        ptp->ccb = NULL;
        ptp->os_err = EIO;
        return -ptp->os_err;
    }

    if (((ccb->ccb_h.status & CAM_STATUS_MASK) == CAM_REQ_CMP) ||
        ((ccb->ccb_h.status & CAM_STATUS_MASK) == CAM_SCSI_STATUS_ERROR)) {
        ptp->scsi_status = ccb->csio.scsi_status;
        ptp->resid = ccb->csio.resid;
        ptp->sense_resid = ccb->csio.sense_resid;

        if ((SAM_STAT_CHECK_CONDITION == ptp->scsi_status) ||
            (SAM_STAT_COMMAND_TERMINATED == ptp->scsi_status)) {
            if (ptp->sense_resid > ptp->sense_len)
                len = ptp->sense_len;   /* crazy; ignore sense_resid */
            else
                len = ptp->sense_len - ptp->sense_resid;
            if (len > 0)
                memcpy(ptp->sense, &(ccb->csio.sense_data), len);
        }
    } else
        ptp->transport_err = 1;

    ptp->cam_dev = fdchan->cam_dev;     // for error processing
    return 0;
}

int
get_scsi_pt_result_category(const struct sg_pt_base * vp)
{
    const struct sg_pt_freebsd_scsi * ptp = &vp->impl;

    if (ptp->os_err)
        return SCSI_PT_RESULT_OS_ERR;
    else if (ptp->transport_err)
        return SCSI_PT_RESULT_TRANSPORT_ERR;
    else if ((SAM_STAT_CHECK_CONDITION == ptp->scsi_status) ||
             (SAM_STAT_COMMAND_TERMINATED == ptp->scsi_status))
        return SCSI_PT_RESULT_SENSE;
    else if (ptp->scsi_status)
        return SCSI_PT_RESULT_STATUS;
    else
        return SCSI_PT_RESULT_GOOD;
}

int
get_scsi_pt_resid(const struct sg_pt_base * vp)
{
    const struct sg_pt_freebsd_scsi * ptp = &vp->impl;

    return ptp->resid;
}

int
get_scsi_pt_status_response(const struct sg_pt_base * vp)
{
    const struct sg_pt_freebsd_scsi * ptp = &vp->impl;

    return ptp->scsi_status;
}

int
get_scsi_pt_sense_len(const struct sg_pt_base * vp)
{
    const struct sg_pt_freebsd_scsi * ptp = &vp->impl;

    if (ptp->sense_resid > ptp->sense_len)
        return ptp->sense_len;  /* strange; ignore ptp->sense_resid */
    else
        return ptp->sense_len - ptp->sense_resid;
}

int
get_scsi_pt_duration_ms(const struct sg_pt_base * vp __attribute__ ((unused)))
{
    // const struct sg_pt_freebsd_scsi * ptp = &vp->impl;

    return -1;
}

int
get_scsi_pt_transport_err(const struct sg_pt_base * vp)
{
    const struct sg_pt_freebsd_scsi * ptp = &vp->impl;

    return ptp->transport_err;
}

int
get_scsi_pt_os_err(const struct sg_pt_base * vp)
{
    const struct sg_pt_freebsd_scsi * ptp = &vp->impl;

    return ptp->os_err;
}

char *
get_scsi_pt_transport_err_str(const struct sg_pt_base * vp, int max_b_len,
                              char * b)
{
    const struct sg_pt_freebsd_scsi * ptp = &vp->impl;

    if (0 == ptp->transport_err) {
        strncpy(b, "no transport error available", max_b_len);
        b[max_b_len - 1] = '\0';
        return b;
    }
#if __FreeBSD_version > 500000
    if (ptp->cam_dev)
        cam_error_string(ptp->cam_dev, ptp->ccb, b, max_b_len, CAM_ESF_ALL,
                         CAM_EPF_ALL);
    else {
        strncpy(b, "no transport error available", max_b_len);
        b[max_b_len - 1] = '\0';
   }
#else
    strncpy(b, "no transport error available", max_b_len);
    b[max_b_len - 1] = '\0';
#endif
    return b;
}

char *
get_scsi_pt_os_err_str(const struct sg_pt_base * vp, int max_b_len, char * b)
{
    const struct sg_pt_freebsd_scsi * ptp = &vp->impl;
    const char * cp;

    cp = safe_strerror(ptp->os_err);
    strncpy(b, cp, max_b_len);
    if ((int)strlen(cp) >= max_b_len)
        b[max_b_len - 1] = '\0';
    return b;
}
