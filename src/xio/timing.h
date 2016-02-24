#include <time.h>

#define SECS	1.0E1
#define	MSECS	1.0E3
#define USECS	1.0E6
#define NSECS	1.0E9

#define TDIFF(tv1, tv2)	(((tv2.tv_sec * NSECS) + (tv2.tv_nsec)) - \
			((tv1.tv_sec * NSECS) + (tv1.tv_nsec)))

#define GET_DIFF_SECS(TDIFF)	(TDIFF/NSECS)
#define GET_DIFF_MSECS(TDIFF)	(TDIFF/USECS)
#define	GET_DIFF_USECS(TDIFF)	(TDIFF/MSECS)

#define GET_SYS_CLOCK(tvar)	clock_gettime(CLOCK_MONOTONIC, tvar)
