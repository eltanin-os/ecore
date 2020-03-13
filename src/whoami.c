#include <tertium/cpu.h>
#include <tertium/std.h>

#include "common.h"

static void
usage(void)
{
	c_ioq_fmt(ioq2, "usage: %s\n", c_std_getprogname());
	c_std_exit(2);
}

ctype_status
main(int argc, char **argv)
{
	char *s;

	c_std_setprogname(argv[0]);

	C_ARGBEGIN {
	default:
		usage();
	} C_ARGEND

	if (argc)
		usage();

	if (!(s = namefromuid(c_sys_geteuid())))
		c_err_diex(1, "uid not found");
	c_ioq_fmt(ioq1, "%s\n", s);
	c_ioq_flush(ioq1);
	return 0;
}
