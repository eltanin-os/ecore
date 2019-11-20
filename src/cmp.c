#include <tertium/cpu.h>
#include <tertium/std.h>

enum {
	LFLAG = 1 << 0,
	SFLAG = 1 << 1,
};

static int
eopen(char *s, uint opts)
{
	ctype_fd fd;

	if ((fd = c_sys_open(s, C_OREAD, 0)) < 0) {
		if (opts & SFLAG)
			c_err_warn("c_sys_open %s", s);
		c_std_exit(2);
	}

	return fd;
}

static int
egetall(ctype_ioq *p, char *s, uint opts, char *b, usize n)
{
	int r;

	if ((r = c_ioq_getall(p, b, n)) < 0) {
		if (opts & SFLAG)
			c_err_warn("c_ioq_getall %s", s);
		c_std_exit(2);
	}

	return r;
}

static void
usage(void)
{
	c_ioq_fmt(ioq2, "usage: %s [-l|-s] file1 file2\n", c_std_getprogname());
	c_std_exit(2);
}

ctype_status
main(int argc, char **argv)
{
	ctype_ioq iqs[2];
	ctype_fd fds[2];
	usize line, bno;
	int r, rval;
	uint opts;
	char bufs[C_BIOSIZ][2];
	char ch[2];

	c_std_setprogname(argv[0]);

	opts = 0;

	C_ARGBEGIN {
	case 'l':
		opts &= ~SFLAG;
		opts |= LFLAG;
		break;
	case 's':
		opts &= ~LFLAG;
		opts |= SFLAG;
		break;
	default:
		usage();
	} C_ARGEND

	if (argc - 2)
		usage();

	fds[0] = C_ISDASH(argv[0]) ? C_FD0 : eopen(argv[0], opts);
	fds[1] = C_ISDASH(argv[1]) ? C_FD1 : eopen(argv[1], opts);
	c_ioq_init(&iqs[0], fds[0], bufs[0], sizeof(bufs[0]), &c_sys_read);
	c_ioq_init(&iqs[1], fds[1], bufs[1], sizeof(bufs[1]), &c_sys_read);

	r = rval = 0;
	bno = line = 0;
	for (;;) {
		if (egetall(&iqs[0], argv[0], opts, &ch[0], 1))
			r += 1;
		if (egetall(&iqs[1], argv[1], opts, &ch[1], 1))
			r += 2;
		if (r)
			break;
		if (ch[0] == ch[1]) {
			if (ch[0] == '\n')
				++line;
		} else if (!(opts & LFLAG)) {
			if (!(opts & SFLAG))
				c_ioq_fmt(ioq1,
				    "%s %s differ: char %ud, line %ud\n",
				    argv[0], argv[1], bno, line);
			rval = 1;
			break;
		} else {
			c_ioq_fmt(ioq1, "%ud %o %o\n", bno, ch[0], ch[1]);
			rval = 1;
		}
		++bno;
	}

	switch (r) {
	case 1:
		rval = c_err_warnx("EOF on %s", argv[0]);
		break;
	case 2:
	case 3:
		rval = c_err_warnx("EOF on %s", argv[1]);
	}

	c_ioq_flush(ioq1);
	return rval;
}
