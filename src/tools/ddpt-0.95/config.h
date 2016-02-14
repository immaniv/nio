/* config.h.  Generated from config.h.in by configure.  */
/* config.h.in.  Generated from configure.ac by autoheader.  */

/* Define to 1 if you have the `clock_gettime' function. */
#define HAVE_CLOCK_GETTIME 1

/* Define to 1 if you have the `fallocate' function. */
#define HAVE_FALLOCATE 1

/* Define to 1 if you have the `fdatasync' function. */
#define HAVE_FDATASYNC 1

/* Define to 1 if you have the `fsync' function. */
#define HAVE_FSYNC 1

/* Define to 1 if you have the `gettimeofday' function. */
#define HAVE_GETTIMEOFDAY 1

/* Define to 1 if you have the `rt' library (-lrt). */
#define HAVE_LIBRT 1

/* Define to 1 if you have the <linux/bsg.h> header file. */
#define HAVE_LINUX_BSG_H 1

/* Define to 1 if you have the <linux/kdev_t.h> header file. */
#define HAVE_LINUX_KDEV_T_H 1

/* Define to 1 if you have the <linux/types.h> header file. */
#define HAVE_LINUX_TYPES_H 1

/* Define to 1 if you have the `nanosleep' function. */
#define HAVE_NANOSLEEP 1

/* Define to 1 if you have the `posix_fadvise' function. */
#define HAVE_POSIX_FADVISE 1

/* Define to 1 if you have the `posix_fallocate' function. */
#define HAVE_POSIX_FALLOCATE 1

/* Define to 1 if you have the `posix_memalign' function. */
#define HAVE_POSIX_MEMALIGN 1

/* Define to 1 if you have the `siginterrupt' function. */
#define HAVE_SIGINTERRUPT 1

/* Define to 1 if you have the `sysconf' function. */
#define HAVE_SYSCONF 1

/* ignore linux bsg */
/* #undef IGNORE_LINUX_BSG */

/* Name of package */
#define PACKAGE "ddpt"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT "dgilbert@interlog.com"

/* Define to the full name of this package. */
#define PACKAGE_NAME "ddpt"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "ddpt 0.95"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "ddpt"

/* Define to the home page for this package. */
#define PACKAGE_URL ""

/* Define to the version of this package. */
#define PACKAGE_VERSION "0.95"

/* ddpt Build Host */
#define SG_LIB_BUILD_HOST "x86_64-unknown-linux-gnu"

/* ddpt on FreeBSD */
/* #undef SG_LIB_FREEBSD */

/* assume ddpt on linux */
#define SG_LIB_LINUX 1

/* also MinGW environment */
/* #undef SG_LIB_MINGW */

/* ddpt on Solaris */
/* #undef SG_LIB_SOLARIS */

/* ddpt on Win32 */
/* #undef SG_LIB_WIN32 */

/* full SCSI sense strings */
#define SG_SCSI_STRINGS 1

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Version number of package */
#define VERSION "0.95"

/* allow large buffers, aligned? */
/* #undef WIN32_SPT_DIRECT */
