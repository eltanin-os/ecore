#include <tertium/cpu.h>
#include <tertium/std.h>

#include "common.h"

#define warn(a, ...)\
((opts & FFLAG) && errno == C_ENOENT) ? 0 : c_err_warn((a), __VA_ARGS__)

enum {
	FFLAG = 1 << 0,
	IFLAG = 1 << 1,
	RFLAG = 1 << 2,
};

static void
usage(void)
{
	c_ioq_fmt(ioq2, "usage: %s [-Rfi] file ...\n", c_std_getprogname());
	c_std_exit(1);
}

int
main(int argc, char **argv)
{
	ctype_dir dir;
	ctype_dent *p;
	int rv;
	uint opts;

	c_std_setprogname(argv[0]);

	opts = 0;

	C_ARGBEGIN {
	case 'R':
		opts |= RFLAG;
		break;
	case 'f':
		opts |= FFLAG;
		break;
	case 'i':
		/* ignore */
		break;
	default:
		usage();
	} C_ARGEND

	if (!argc) {
		if (!(opts & FFLAG))
			usage();
		c_std_exit(0);
	}

	if (c_dir_open(&dir, argv, 0, nil) < 0)
		c_err_die(1, "c_dir_open");

	rv = 0;

	while ((p = c_dir_read(&dir))) {
		switch (p->info) {
		case C_FSD:
			if (!(opts & RFLAG)) {
				c_dir_set(&dir, p, C_FSSKP);
				rv = c_err_warnx("%s: %s",
				    p->path, serr(C_EISDIR));
				continue;
			}
			break;
		case C_FSDP:
			if (!(opts & RFLAG))
				continue;
			if (c_sys_rmdir(p->path) < 0)
				rv = c_err_warn("c_sys_rmdir %s", p->path);
			break;
		case C_FSERR:
			rv = c_err_warnx("%s: %s", p->path, serr(p->err));
			break;
		default:
			if (c_sys_unlink(p->path) < 0)
				rv = c_err_warn("c_sys_unlink %s", p->path);
		}
	}

	c_dir_close(&dir);

	return rv;
}
