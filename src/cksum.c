#include <tertium/cpu.h>
#include <tertium/std.h>

static void
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
		return;

	if (c_sys_fstat(&st, fd) < 0)
		return;

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
}

static void
usage(void)
{
	c_ioq_fmt(ioq2, "usage: %s [file ...]\n", c_std_getprogname());
	c_ioq_flush(ioq2);
	c_std_exit(1);
}

int
main(int argc, char **argv)
{
	c_std_setprogname(argv[0]);

	C_ARGBEGIN {
	default:
		usage();
	} C_ARGEND

	if (!argc)
		cksum(nil);

	for (; *argv; argc--, argv++) {
		if (C_ISDASH(*argv))
			cksum(nil);
		else
			cksum(*argv);
	}

	c_ioq_flush(ioq1);

	return 0;
}
