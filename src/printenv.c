#include <tertium/cpu.h>
#include <tertium/std.h>

static void
usage(void)
{
	c_ioq_fmt(ioq2, "usage: %s [name]", c_std_getprogname());
	c_std_exit(1);
}

ctype_status
main(int argc, char **argv)
{
	char *s;

	c_std_setprogname(argv[0]);
	--argc, ++argv;

	if (c_std_noopt(argmain, *argv)) usage();
	argc -= argmain->idx;
	argv -= argmain->idx;
	switch (argc) {
	case 0:
		for (; *environ; ++environ) c_ioq_fmt(ioq1, "%s\n", *environ);
		break;
	case 1:
		if (!(s = c_std_getenv(*argv))) c_std_exit(1);
		c_ioq_fmt(ioq1, "%s\n", s);
		break;
	default:
		usage();
	}
	c_ioq_flush(ioq1);
	return 0;
}
