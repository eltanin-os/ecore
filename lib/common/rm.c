#include <tertium/cpu.h>
#include <tertium/std.h>

#include "common.h"

ctype_status
remove(char **argv, uint opts)
{
	ctype_dir dir;
	ctype_dent *p;
	int rv;

	if (c_dir_open(&dir, argv, 0, nil) < 0)
		c_err_die(1, "c_dir_open");

	rv = 0;

	while ((p = c_dir_read(&dir))) {
		switch (p->info) {
		case C_FSD:
			if (!(opts & RM_RFLAG)) {
				c_dir_set(&dir, p, C_FSSKP);
				rv = c_err_warnx("%s: %s",
				    p->path, serr(C_EISDIR));
				continue;
			}
			break;
		case C_FSDP:
			if (!(opts & RM_RFLAG))
				continue;
			if (c_sys_rmdir(p->path) < 0)
				rv = c_err_warn("c_sys_rmdir %s", p->path);
			break;
		case C_FSERR:
			rv = c_err_warnx("%s: %s", p->path, serr(p->err));
			break;
		default:
			if (c_sys_unlink(p->path) < 0 &&
			    ((opts & RM_FFLAG) && errno != C_ENOENT))
				rv = c_err_warn("c_sys_unlink %s", p->path);
		}
	}

	c_dir_close(&dir);

	return rv;
}
