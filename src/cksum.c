#include <tertium/cpu.h>
#include <tertium/std.h>

#include "common.h"

static void
usage(void)
{
	c_ioq_fmt(ioq2, "usage: %s [file ...]\n", c_std_getprogname());
	c_std_exit(1);
}

int
main(int argc, char **argv)
{
	ctype_hst hs;
	int rv;

	c_std_setprogname(argv[0]);

	C_ARGBEGIN {
	default:
		usage();
	} C_ARGEND

	if (!argc)
		argv = tmpargv("-");

	rv = 0;

	for (; *argv; --argc, ++argv) {
		if (C_ISDASH(*argv))
			*argv = "<stdin>";
		if (c_hsh_putfile(&hs, c_hsh_crc32p, *argv) < 0)
			rv = c_err_warn("c_hsh_putfile %s", *argv);
		c_ioq_fmt(ioq1, "%d %d %s\n", c_hsh_state0(&hs), hs.len, *argv);
	}

	c_ioq_flush(ioq1);

	return rv;
}
