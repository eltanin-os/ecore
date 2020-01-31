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
	int r;
	char buf[C_H32GEN_DIGEST];

	c_std_setprogname(argv[0]);

	C_ARGBEGIN {
	default:
		usage();
	} C_ARGEND

	if (!argc)
		argv = tmpargv("-");

	r = 0;
	for (; *argv; --argc, ++argv) {
		if (C_ISDASH(*argv))
			*argv = "<stdin>";
		c_hsh_crc32p->init(&hs);
		if (c_hsh_putfile(&hs, c_hsh_crc32p, *argv) < 0)
			r = c_err_warn("c_hsh_putfile %s", *argv);
		c_hsh_crc32p->end(&hs);
		c_hsh_digest(&hs, c_hsh_crc32p, buf);
		c_ioq_fmt(ioq1, "%ud %d %s\n",
		    c_uint_32unpack(buf), c_hsh_len(&hs), *argv);
	}
	c_ioq_flush(ioq1);
	return r;
}
