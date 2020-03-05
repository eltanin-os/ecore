#include <tertium/cpu.h>
#include <tertium/std.h>

#include "common.h"

static ctype_status
rmpath(char *dir)
{
	char *s;

	for (;;) {
		if (c_sys_rmdir(dir) < 0)
			return c_err_warn("c_sys_rmdir %s", dir);
		if (!(s = c_str_rchr(dir, C_USIZEMAX, '/')))
			break;
		*s = 0;
	}
	return 0;
}

static void
usage(void)
{
	c_ioq_fmt(ioq2, "usage: %s [-p] dir ...\n", c_std_getprogname());
	c_std_exit(1);
}

ctype_status
main(int argc, char **argv)
{
	ctype_status r;
	int pflag;

	c_std_setprogname(argv[0]);

	pflag = 0;

	C_ARGBEGIN {
	case 'p':
		pflag = 1;
		break;
	default:
		usage();
	} C_ARGEND

	if (!argc)
		usage();

	for (; *argv; --argc, ++argv) {
		trim_trailing_slash(*argv);
		if (pflag)
			r |= rmpath(*argv);
		else if (c_sys_rmdir(*argv) < 0)
			r = c_err_warn("c_sys_rmdir %s", *argv);
	}
	return r;
}
