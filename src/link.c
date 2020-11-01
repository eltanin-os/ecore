#include <tertium/cpu.h>
#include <tertium/std.h>

static void
usage(void)
{
	c_ioq_fmt(ioq2, "usage: %s source target\n", c_std_getprogname());
	c_std_exit(1);
}

ctype_status
main(int argc, char **argv)
{
	c_std_setprogname(argv[0]);
	--argc, ++argv;

	if (c_std_noopt(argmain, *argv))
		usage();
	argc -= argmain->idx;
	argv += argmain->idx;

	if (argc - 2)
		usage();

	if (c_sys_link(argv[0], argv[1]))
		c_err_die(1, "c_sys_link %s %s", argv[0], argv[1]);

	return 0;
}
