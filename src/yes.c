#include <tertium/cpu.h>
#include <tertium/std.h>

#include "common.h"

static void
usage(void)
{
	c_ioq_fmt(ioq2, "usage: %s [expletive]\n", c_std_getprogname());
	c_std_exit(1);
}

int
main(int argc, char **argv)
{
	ctype_arr *p;
	char *s;

	c_std_setprogname(argv[0]);

	C_ARGBEGIN {
	default:
		usage();
	} C_ARGEND

	s = argc ? *argv : "y";
	c_ioq_set(ioq1, C_IOQ_ONOFLUSH);
	while (c_ioq_fmt(ioq1, "%s\n", s) > 0) ;

	p = c_ioq_arr(ioq1);
	for (;;)
		c_std_allrw(c_sys_write, C_FD1, c_arr_data(p), c_arr_bytes(p));

	/* NOT REACHED */
	return 1;
}
