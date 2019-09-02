#include <tertium/cpu.h>
#include <tertium/std.h>

#include "common.h"

enum {
	FFLAG = 1 << 0,
	SFLAG = 1 << 1,
	LFLAG = 1 << 2,
};

static int
links(char *src, char *dest, uint opts)
{
	if (opts & FFLAG)
		c_sys_unlink(dest);

	if (opts & SFLAG) {
		if (c_sys_symlink(src, dest) < 0)
			return c_err_warn("c_sys_symlink %s %s", src, dest);
	} else if (opts & LFLAG) {
		if (c_sys_llink(src, dest) < 0)
			return c_err_warn("c_sys_link %s %s", src, dest);
	} else {
		if (c_sys_link(src, dest) < 0)
			return c_err_warn("c_sys_llink %s %s", src, dest);
	}

	return 0;
}

static void
usage(void)
{
	c_ioq_fmt(ioq2,
	    "usage: %s [-f] [-L|-P|-s] source [target]\n"
	    "       %s [-f] [-L|-P|-s] source ... directory\n",
	    c_std_getprogname(), c_std_getprogname());
	c_std_exit(1);
}

int
main(int argc, char **argv)
{
	CStat st;
	int   rv;
	uint  opts;
	char *dest;

	c_std_setprogname(argv[0]);

	opts = 0;

	C_ARGBEGIN {
	case 'L':
		opts &= ~SFLAG;
		opts |= LFLAG;
		break;
	case 'P':
		opts &= ~(LFLAG|SFLAG);
		break;
	case 'f':
		opts |= FFLAG;
		break;
	case 's':
		opts &= ~LFLAG;
		opts |= SFLAG;
		break;
	default:
		usage();
	} C_ARGEND

	switch (argc) {
	case 0:
		usage();
	case 1:
		c_std_exit(links(argv[0], pathcat(argv[0], ".", 0), opts));
	case 2:
		c_std_exit(links(argv[0], pathcat(argv[0], argv[1], 0), opts));
	}

	dest = argv[argc - 1];
	argv[argc - 1] = nil;
	if (c_sys_stat(&st, dest) < 0)
		c_err_die(1, "c_sys_stat %s", dest);

	if (!C_ISDIR(st.mode))
		usage();

	rv = 0;

	for (; *argv; argc--, argv++)
		rv |= links(*argv, pathcat(*argv, dest, 1), opts);

	return rv;
}
