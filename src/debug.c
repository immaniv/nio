#include <stdio.h>
#include "debug.h"


void dbg_printf(int verbosity, const char *format, ...) 
{
#ifdef DEBUG
	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
#endif
}
	
