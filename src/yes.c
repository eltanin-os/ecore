#include <tertium/cpu.h>
#include <tertium/std.h>

#include "common.h"

static void
usage(void)
{
	c_ioq_fmt(ioq2, "usage: %s [expletive]\n", c_std_getprogname());
	c_std_exit(1);
}

ctype_status
main(int argc, char **argv)
{
	ctype_arr arr;
	char *s;
	char buf[C_BIOSIZ];

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

	s = argc ? *argv : "y";
	c_arr_init(&arr, buf, sizeof(buf));
	while (c_arr_fmt(&arr, "%s\n", s) > 0) ;

	for (;;)
		c_std_allrw(c_sys_write, C_FD1,
		    c_arr_data(&arr), c_arr_bytes(&arr));

	/* NOT REACHED */
	return 1;
}
