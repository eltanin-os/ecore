#include <tertium/cpu.h>
#include <tertium/std.h>

#include "common.h"

static uint ropts;

static int
chownf(Dir *p, char *path, uint uid)
{
	CDir dir;
	int r, rv;

	switch (dir_open(p, &dir, path, ropts)) {
	case -1:
		return c_err_warn("dir_open %s", path);
	case  1:
		if (c_sys_chown(path, uid, p->dp->info.st_gid) < 0)
			return c_err_warn("c_sys_chown %s", path);
		/* fallthrough */
	case  2:
		return 0;
	}

	if (c_sys_chown(path, uid, p->dp->info.st_gid) < 0)
		return c_err_warn("c_sys_chown %s", path);

	rv = 0;
	p->depth++;
	while ((r = c_dir_read(p->dp, &dir)) > 0) {
		if (C_ISDIR(p->dp->info.st_mode)) {
			if (chownf(p, p->dp->path, uid)) {
				rv = 1;
				continue;
			}
		} else {
			if (c_sys_chown(p->dp->path, uid, p->dp->info.st_gid) < 0)
				return c_err_warn("c_sys_chown %s", path);
		}
	}
	p->depth--;

	if (r < 0)
		rv = c_err_warn("dir_read %s", dir.path);

	dir_close(p, &dir);

	return 0;
}

static void
usage(void)
{
	c_ioq_fmt(ioq2, "usage: %s [-R [-H|-L|-P]] owner[:group] file ...\n",
	    c_std_getprogname());
	c_std_exit(1);
}

int
main(int argc, char **argv)
{
	Dir  dir;
	int  rv;
	uint uid;

	c_std_setprogname(argv[0]);

	c_mem_set(&dir, sizeof(dir), 0);
	dir.maxdepth = 1;

	C_ARGBEGIN {
	case 'R':
		dir.maxdepth = 0;
		break;
	case 'H':
		ropts |= C_FSCOM;
		break;
	case 'L':
		ropts &= ~C_FSPHY;
		ropts |= C_FSLOG;
		break;
	case 'P':
		ropts &= ~C_FSLOG;
		ropts |= C_FSPHY;
		break;
	default:
		usage();
	} C_ARGEND

	if (argc < 2)
		usage();

	uid = estrtovl(argv[0], 0, 0, C_UINTMAX);
	rv  = 0;

	argv++;
	for (; *argv; argc--, argv++)
		rv |= chownf(&dir, *argv, uid);

	return rv;
}
