#include <tertium/cpu.h>
#include <tertium/std.h>

static void
usage(void)
{
	c_ioq_fmt(ioq2, "usage: %s string\n", c_std_getprogname());
	c_std_exit(1);
}

ctype_status
main(int argc, char **argv)
{
	c_std_setprogname(argv[0]);
	--argc, ++argv;

	if (c_std_noopt(argmain, *argv)) usage();
	argc -= argmain->idx;
	argv += argmain->idx;
	if (argc - 1) usage();

	c_ioq_fmt(ioq1, "%s\n", c_gen_dirname(*argv));
	c_ioq_flush(ioq1);
	return 0;
}
