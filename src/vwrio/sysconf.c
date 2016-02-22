#include <unistd.h>

#include "sysconf.h"

int 
sys_conf(struct sys_info *syscfg) 
{
#ifdef LINUX
	syscfg->num_cpus = sysconf(_SC_NPROCESSORS_ONLN);
	syscfg->phy_pgsize = sysconf(_SC_PAGESIZE);
	syscfg->num_phy_pages = sysconf(_SC_PHYS_PAGES);
	syscfg->av_phy_pages = sysconf(_SC_AVPHYS_PAGES);
	syscfg->clock_tck = sysconf(_SC_CLK_TCK);
#endif
	return 0;
}
