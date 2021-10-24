#include <tertium/cpu.h>
#include <tertium/std.h>

#include "common.h"

static void
usage(void)
{
	c_ioq_fmt(ioq2, "usage: %s [-p] dir ...\n", c_std_getprogname());
	c_std_exit(1);
}

ctype_status
main(int argc, char **argv)
{
	ctype_status (*func)(char *);
	ctype_status r;

	c_std_setprogname(argv[0]);
	--argc, ++argv;

	func = c_nix_rmdir;

	while (c_std_getopt(argmain, argc, argv, "p")) {
		switch (argmain->opt) {
		case 'p':
			func = c_nix_rmpath;
			break;
		}
	}
	argc -= argmain->idx;
	argv += argmain->idx;

	if (!argc)
		usage();

	r = 0;
	for (; *argv; --argc, ++argv) {
		if (func(*argv) < 0)
			r = c_err_warn("remove %s", *argv);
	}
	return r;
}
