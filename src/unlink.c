#include <tertium/cpu.h>
#include <tertium/std.h>

static void
usage(void)
{
	c_ioq_fmt(ioq2, "usage: %s file", c_std_getprogname());
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

	if (argc-1)
		usage();

	if (c_sys_unlink(*argv) < 0)
		c_err_die(1, "c_sys_unlink %s", *argv);

	return 0;
}
