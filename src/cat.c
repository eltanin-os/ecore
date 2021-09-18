#include <tertium/cpu.h>
#include <tertium/std.h>

#include "common.h"

static void
usage(void)
{
	c_ioq_fmt(ioq2, "usage: %s [-u] [file ...]\n", c_std_getprogname());
	c_std_exit(1);
}

ctype_status
main(int argc, char **argv)
{
	ctype_status r;

	c_std_setprogname(argv[0]);
	--argc, ++argv;

	while (c_std_getopt(argmain, argc, argv, "u")) {
		switch (argmain->opt) {
		case 'u':
			c_ioq_init(ioq1, C_FD1, nil, 0, &c_nix_fdwrite);
			break;
		default:
			usage();
		}
	}
	argc -= argmain->idx;
	argv += argmain->idx;

	if (!argc)
		argv = tmpargv("-");

	r = 0;
	for (; *argv; --argc, ++argv) {
		if (C_ISDASH(*argv))
			*argv = "<stdin>";
		if (c_ioq_putfile(ioq1, *argv) < 0)
			r = c_err_warn("putfile %s", *argv);
	}
	c_ioq_flush(ioq1);
	return r;
}
