#include "common.h"

long int
tdiff(struct timespec t1, struct timespec t2) 
{
	return (((t2.tv_sec * 1000000000) + (t2.tv_nsec)) - \
		((t1.tv_sec * 1000000000) + (t1.tv_nsec)));
}
