#include <tertium/cpu.h>
#include <tertium/std.h>

static void
usage(void)
{
	c_ioq_fmt(ioq2, "usage: %s file", c_std_getprogname());
	c_std_exit(1);
}

ctype_status
main(int argc, char **argv)
{
	c_std_setprogname(argv[0]);
	--argc, ++argv;

	while (c_std_getopt(argmain, argc, argv, "")) {
		switch (argmain->opt) {
		default:
			usage();
		}
	}
	argc -= argmain->idx;
	argv += argmain->idx;

	if (argc - 1)
		usage();

	if (c_sys_unlink(*argv) < 0)
		c_err_die(1, "c_sys_unlink %s", *argv);

	return 0;
}
