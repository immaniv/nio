#include <time.h>

#include "timer.h"
#include "common.h"


extern struct timespec start, end;

void start_sys_timer() 
{
	clock_gettime(CLOCK_MONOTONIC, &start);
}

void end_sys_timer() 
{
	clock_gettime(CLOCK_MONOTONIC, &end);
}

long int sys_timer_count() {
	return tdiff(start, end);
}

