#include <tertium/cpu.h>
#include <tertium/std.h>

#include "common.h"

static void
usage(void)
{
	c_ioq_fmt(ioq2, "usage: %s [-m mode] file ...\n", c_std_getprogname());
	c_std_exit(1);
}

ctype_status
main(int argc, char **argv)
{
	ctype_status r;
	uint mode;

	c_std_setprogname(argv[0]);
	--argc, ++argv;

	mode = C_DEFFILEMODE;

	while (c_std_getopt(argmain, argc, argv, "m:")) {
		switch (argmain->opt) {
		case 'm':
			mode = strtomode(argmain->arg, C_ACCESSPERMS, 0);
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
	for (; *argv; ++argv)
		if (c_sys_mknod(*argv, C_IFIFO | mode, 0) < 0)
			c_err_warn("c_sys_mknod %s", *argv);
	return r;
}
