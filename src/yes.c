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
	char buf[C_IOQ_BSIZ];

	c_std_setprogname(argv[0]);
	--argc, ++argv;

	if (c_std_noopt(argmain, *argv)) usage();
	argc -= argmain->idx;
	argv += argmain->idx;

	s = argc ? *argv : "y";
	c_arr_init(&arr, buf, sizeof(buf));
	while (c_arr_fmt(&arr, "%s\n", s) > 0) ;

	for (;;) {
		c_nix_allrw(c_nix_fdwrite, C_IOQ_FD1,
		    c_arr_data(&arr), c_arr_bytes(&arr));
	}
	/* NOT REACHED */
	return 1;
}
