#include <tertium/cpu.h>
#include <tertium/std.h>

static void
usage(void)
{
	c_ioq_fmt(ioq2, "usage: %s source target\n", c_std_getprogname());
	c_std_exit(1);
}

int
main(int argc, char **argv)
{
	c_std_setprogname(argv[0]);

	C_ARGBEGIN {
	default:
		usage();
	} C_ARGEND

	if (argc - 2)
		usage();

	if (c_sys_link(argv[0], argv[1]))
		c_err_die(1, "c_sys_link %s %s", argv[0], argv[1]);

	return 0;
}
