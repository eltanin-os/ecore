#include <tertium/cpu.h>
#include <tertium/std.h>

#include "common.h"

static void
usage(void)
{
	c_ioq_fmt(ioq2, "usage: %s [-ifRr] file ...\n", c_std_getprogname());
	c_std_exit(1);
}

ctype_status
main(int argc, char **argv)
{
	uint opts;

	c_std_setprogname(argv[0]);
	--argc, ++argv;

	opts = 0;

	while (c_std_getopt(argmain, argc, argv, "Rrfi")) {
		switch (argmain->opt) {
		case 'R':
		case 'r':
			opts |= RM_RFLAG;
			break;
		case 'f':
			opts |= RM_FFLAG;
			break;
		case 'i':
			opts |= RM_IFLAG;
			break;
		default:
			usage();
		}
	}
	argc -= argmain->idx;
	argv += argmain->idx;

	if (!argc) {
		if (!(opts & RM_FFLAG))
			usage();
		c_std_exit(0);
	}
	return remove(argv, opts);
}
