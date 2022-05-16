#include <tertium/cpu.h>
#include <tertium/std.h>

#include "common.h"

static void
usage(void)
{
	c_ioq_fmt(ioq2, "usage: %s [file ...]\n", c_std_getprogname());
	c_std_exit(1);
}

ctype_status
main(int argc, char **argv)
{
	ctype_hst hs;
	ctype_status r;
	char buf[C_HSH_H32DIG];

	c_std_setprogname(argv[0]);
	--argc, ++argv;

	if (c_std_noopt(argmain, *argv)) usage();
	argc -= argmain->idx;
	argv += argmain->idx;
	if (!argc) argv = tmpargv("-");

	r = 0;
	for (; *argv; ++argv) {
		if (C_STD_ISDASH(*argv))
			*argv = "<stdin>";
		c_hsh_crc32p->init(&hs);
		if (c_hsh_putfile(&hs, c_hsh_crc32p, *argv) < 0)
			r = c_err_warn("c_hsh_putfile %s", *argv);
		c_hsh_crc32p->end(&hs, buf);
		c_ioq_fmt(ioq1, "%d %d %s\n",
		    c_uint_32unpack(buf), c_hsh_octets(&hs), *argv);
	}
	c_ioq_flush(ioq1);
	return r;
}
