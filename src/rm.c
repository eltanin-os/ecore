#include <tertium/cpu.h>
#include <tertium/std.h>

#include "common.h"

#define warns(a, ...)\
((opts & FFLAG) && errno == C_ENOENT) ? 0 : c_err_warn((a), __VA_ARGS__)

enum {
	FFLAG = 1 << 0,
	IFLAG = 1 << 1,
};

static uint opts;

static int
rm(Dir *p, char *path)
{
	CDir dir;
	int r, rv;

	switch (dir_open(p, &dir, path, 0)) {
	case -1:
		return warns("dir_open %s", path);
	case  1:
		if (c_sys_unlink(path) < 0)
			return warns("c_sys_unlink %s", path);
		return 0;
	}

	rv = 0;
	p->depth++;
	while ((r = c_dir_read(p->dp, &dir)) > 0) {
		if (C_ISDIR(p->dp->info.st_mode)) {
			if (rm(p, p->dp->path) < 0) {
				rv = 1;
				continue;
			}
		} else {
			if (c_sys_unlink(p->dp->path)) {
				rv = c_err_warn("c_sys_unlink %s", p->dp->path);
				continue;
			}
		}
	}
	p->depth--;

	dir_close(p, &dir);

	if (r < 0)
		rv = c_err_warn("c_dir_read %s", dir.path);

	if (c_sys_rmdir(dir.path) < 0)
		return c_err_warn("c_sys_rmdir %s", dir.path);

	return rv;
}

static void
usage(void)
{
	c_ioq_fmt(ioq2, "usage: %s [-Rfi] file ...\n", c_std_getprogname());
	c_std_exit(1);
}

int
main(int argc, char **argv)
{
	Dir dir;
	int rv;

	c_std_setprogname(argv[0]);

	c_mem_set(&dir, sizeof(dir), 0);
	dir.hp = (void *)-1;
	dir.maxdepth = 1;

	C_ARGBEGIN {
	case 'R':
		dir.maxdepth = 0;
		break;
	case 'f':
		opts |= FFLAG;
		break;
	case 'i':
		opts |= IFLAG;
		break;
	default:
		usage();
	} C_ARGEND

	if (!argc) {
		if (!(opts & FFLAG))
			usage();
		c_std_exit(0);
	}

	rv = 0;

	for (; *argv; argc--, argv++)
		rv |= rm(&dir, *argv);

	return rv;
}
