
struct sys_info {
	long num_cpus;
	long phy_pgsize;
	long num_phy_pages;
	long av_phy_pages;
	long clock_tck;
};

struct mem {
	struct cpu *local_cpu;
	int pgsize;
	long av_mem;
};

int sys_conf (struct sys_info *sys_config);
