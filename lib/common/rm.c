#include <tertium/cpu.h>
#include <tertium/std.h>

#include "common.h"

ctype_status
remove(char **argv, uint opts)
{
	ctype_dir dir;
	ctype_dent *p;
	ctype_status r;

	if (c_dir_open(&dir, argv, 0, nil) < 0)
		c_err_die(1, "c_dir_open");

	r = 0;
	while ((p = c_dir_read(&dir))) {
		if (p->info == C_FSNS) {
			if (!(opts & RM_FFLAG)) {
				errno = C_ENOENT;
				r = c_err_warn("remove %s", p->path);
			}
			continue;
		}
		if ((opts & RM_IFLAG) && yesno("remove", p->path))
			continue;
		switch (p->info) {
		case C_FSD:
			if (!(opts & RM_RFLAG)) {
				c_dir_set(&dir, p, C_FSSKP);
				r = c_err_warnx("%s: %r", p->path, C_EISDIR);
				continue;
			}
			break;
		case C_FSDP:
			if (!(opts & RM_RFLAG))
				continue;
			if (c_sys_rmdir(p->path) < 0)
				r = c_err_warn("c_sys_rmdir %s", p->path);
			break;
		case C_FSERR:
			r = c_err_warnx("%s: %r", p->path, p->err);
			break;
		default:
			if (c_sys_unlink(p->path) < 0)
				r = c_err_warn("c_sys_unlink %s", p->path);
		}
	}
	c_dir_close(&dir);
	return r;
}
