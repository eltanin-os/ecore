#include <tertium/cpu.h>
#include <tertium/std.h>

#include "common.h"

static CDent __dir_entry;

int
dir_open(Dir *p, CDir *dp, char *path, uint opts)
{
	CArr  arr;
	int (*stf)(CStat *, char *);

	p->dp = &__dir_entry;

	stf = C_FSFLW(opts, p->depth) ? c_sys_stat : c_sys_lstat;
	if (stf(&p->dp->info, path) < 0)
		return -1;

	if (!(p->hp == (void *)-1))
		switch (c_dir_hist(&p->hp, &p->dp->info)) {
		case -1:
			c_err_die(1, "c_dir_hist");
		case  1:
			return 2;
		}

	errno = 0;
	if ((p->maxdepth && p->depth+1 >= p->maxdepth) ||
	    c_dir_open(dp, path, opts) < 0) {
		if (errno && errno != C_ENOTDIR)
			return -1;
		c_arr_init(&arr, p->dp->path, sizeof(p->dp->path));
		if (c_arr_fmt(&arr, "%s", path) < 0)
			return -1;
		p->dp->plen = p->dp->nlen = c_arr_bytes(&arr);
		return 1;
	}

	return 0;
}

void
dir_close(Dir *p, CDir *dp)
{
	c_dir_close(dp);
	if (!p->depth) {
		if (!(p->hp == (void *)-1))
			while (p->hp)
				c_dst_lfree(c_dst_lpop(&p->hp));
	}
}
