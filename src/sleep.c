#include <tertium/cpu.h>
#include <tertium/std.h>

#include "common.h"

static void
usage(void)
{
	c_ioq_fmt(ioq2, "usage: %s seconds\n", c_std_getprogname());
	c_std_exit(1);
}

ctype_status
main(int argc, char **argv)
{
	c_std_setprogname(argv[0]);
	--argc, ++argv;

	if (c_std_noopt(argmain, *argv)) {
		if (c_utf8_isdigit(argmain->opt)) {
			errno = C_ERANGE;
			c_err_die(1, nil);
		}
		usage();
	}
	argc -= argmain->idx;
	argv += argmain->idx;

	if (argc - 1)
		usage();

	deepsleep(estrtouvl(*argv, 0, 0, C_UVLONGMAX));
	return 0;
}
