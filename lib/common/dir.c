#include <tertium/cpu.h>
#include <tertium/std.h>

#include "common.h"

static CDent __dir_entry;

int
dir_open(Dir *p, CDir *dp, char *path, uint opts)
{
	CArr  arr;
	CStat st;
	int (*stf)(CStat *, char *);

	p->dp = &__dir_entry;

	stf = C_FSFLW(opts, p->depth) ? c_sys_stat : c_sys_lstat;
	if (stf(&st, path) < 0)
		return -c_err_warn("c_sys_(l)stat %s", path);

	switch (c_dir_hist(&p->hp, &st)) {
	case -1:
		return -c_err_warn("c_dir_hist");
	case  1:
		return 2;
	}

	if (c_dir_open(dp, path, opts) < 0) {
		if (errno != C_ENOTDIR)
			return -c_err_warn("c_dir_open %s", path);
		c_arr_init(&arr, p->dp->path, sizeof(p->dp->path));
		if (c_arr_cats(&arr, path) < 0)
			return -c_err_warn("c_arr_cats %s", path);
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
		while (p->hp)
			c_dst_lfree(c_dst_lpop(&p->hp));
	}
}
