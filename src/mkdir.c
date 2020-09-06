#include <tertium/cpu.h>
#include <tertium/std.h>

#include "common.h"

static void
usage(void)
{
	c_ioq_fmt(ioq2, "usage: %s [-p] [-m mode] dir ...\n",
	    c_std_getprogname());
	c_std_exit(1);
}

ctype_status
main(int argc, char **argv)
{
	ctype_status r;
	int pflag;
	uint dmode, mode;
	uint mask;

	c_std_setprogname(argv[0]);
	--argc, ++argv;

	mask = c_sys_umask(0);
	mode = C_ACCESSPERMS & ~mask;
	dmode = mode | C_IWUSR | C_IXUSR;
	pflag = 0;

	while (c_std_getopt(argmain, argc, argv, "pm:")) {
		switch (argmain->opt) {
		case 'p':
			pflag = 1;
			break;
		case 'm':
			mode = strtomode(argmain->arg, C_ACCESSPERMS, mask);
			break;
		default:
			usage();
		}
	}
	argc -= argmain->idx;
	argv += argmain->idx;

	if (!argc)
		usage();

	r = 0;
	for (; *argv; --argc, ++argv) {
		trim_trailing_slash(*argv);
		if (pflag)
			r |= mkpath(*argv, mode, dmode);
		else if (c_sys_mkdir(*argv, mode) < 0)
			r = c_err_warn("c_sys_mkdir %s", *argv);
	}
	return r;
}
