#include <tertium/cpu.h>
#include <tertium/std.h>

#include "common.h"

static int
cksum(char *s)
{
	CH32st hs;
	CStat  st;
	usize  n;
	size   rf;
	int    fd;
	char   buf[C_BIOSIZ];

	fd = C_FD0;
	n  = 0;

	if (!s) {
		s = "<stdin>";
		goto fallback;
	}

	if ((fd = c_sys_open(s, C_OREAD, 0)) < 0)
		return c_err_warn("c_sys_open %s", s);

	if (c_sys_fstat(&st, fd) < 0)
		return c_err_warn("c_sys_fstat %s", s);

	n    = st.st_size;
	hs.a = c_hsh_putfd(c_hsh_crc32p, fd, n);

	goto done;
fallback:
	c_hsh_crc32p->init(&hs);
	while ((rf = c_sys_read(fd, buf, sizeof(buf))) > 0) {
		c_hsh_crc32p->update(&hs, buf, rf);
		n += rf;
	}
	c_hsh_crc32p->end(&hs);
done:
	c_ioq_fmt(ioq1, "%ud %d %s\n", hs.a, n, s);
	return 0;
}

static void
usage(void)
{
	c_ioq_fmt(ioq2, "usage: %s [file ...]\n", c_std_getprogname());
	c_std_exit(1);
}

int
main(int argc, char **argv)
{
	int rv;

	c_std_setprogname(argv[0]);

	C_ARGBEGIN {
	default:
		usage();
	} C_ARGEND

	rv = 0;

	if (!argc) {
		argv[0] = "-";
		argv[1] = nil;
	}

	for (; *argv; argc--, argv++) {
		if (C_ISDASH(*argv))
			*argv = nil;
		rv |= cksum(*argv);
	}

	c_ioq_flush(ioq1);

	return rv;
}
