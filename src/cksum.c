#include <tertium/cpu.h>
#include <tertium/std.h>

static void
usage(void)
{
	c_ioq_fmt(ioq2, "usage: %s [file ...]\n", c_std_getprogname());
	c_std_exit(1);
}

int
main(int argc, char **argv)
{
	CH32st hs;
	int rv;

	c_std_setprogname(argv[0]);

	C_ARGBEGIN {
	default:
		usage();
	} C_ARGEND

	if (!argc) {
		argv[0] = "-";
		argv[1] = nil;
	}

	rv = 0;

	for (; *argv; argc--, argv++) {
		if (C_ISDASH(*argv))
			*argv = "<stdin>";
		if (c_hsh_putfile(&hs, c_hsh_crc32p, *argv) < 0)
			rv = c_err_warn("c_hsh_putfile %s", *argv);
		c_ioq_fmt(ioq1, "%ud %d %s\n", hs.state[0], hs.len, *argv);
	}

	c_ioq_flush(ioq1);

	return rv;
}
