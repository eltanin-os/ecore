#include <tertium/cpu.h>
#include <tertium/std.h>

static int
fd0sum(char *s)
{
	CH32st hs;
	size   rf;
	usize  n;
	char   buf[C_BIOSIZ];

	n = 0;

	c_hsh_crc32p->init(&hs);
	while ((rf = c_sys_read(C_FD0, buf, sizeof(buf))) > 0) {
		c_hsh_crc32p->update(&hs, buf, rf);
		n += rf;
	}
	c_hsh_crc32p->end(&hs);
	c_ioq_fmt(ioq1, "%ud %d %s\n", hs.a, n, "<stdin>");

	return 0;
}

static int
fdsum(char *s)
{
	CStat  st;
	int    fd;
	u32int hsh;

	if ((fd = c_sys_open(s, C_OREAD, 0)) < 0)
		return c_err_warn("c_sys_open %s", s);

	if (c_sys_fstat(&st, fd) < 0)
		return c_err_warn("c_sys_fstat %s", s);

	hsh = c_hsh_putfd(c_hsh_crc32p, fd, st.st_size);
	c_ioq_fmt(ioq1, "%ud %d %s\n", hsh, st.st_size, s);

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
	int (*cfn)(char *);
	int   rv;

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
		cfn = C_ISDASH(*argv) ? fd0sum : fdsum;
		rv |= cfn(*argv);
	}

	c_ioq_flush(ioq1);

	return rv;
}
