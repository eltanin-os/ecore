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
		if (opts & RM_IFLAG) {
			c_ioq_fmt(ioq2, "%s: remove '%s'? ",
			    c_std_getprogname(), p->path);
			c_ioq_flush(ioq2);
			if (yesno())
				continue;
		}
		switch (p->info) {
		case C_FSD:
			if (!(opts & RM_RFLAG)) {
				c_dir_set(&dir, p, C_FSSKP);
				r = c_err_warnx("%s: %s",
				    p->path, serr(C_EISDIR));
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
			r = c_err_warnx("%s: %s", p->path, serr(p->err));
			break;
		default:
			if (c_sys_unlink(p->path) < 0 &&
			    (!(opts & RM_FFLAG) || errno != C_ENOENT))
				r = c_err_warn("c_sys_unlink %s", p->path);
		}
	}
	c_dir_close(&dir);
	return r;
}
