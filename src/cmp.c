#include <tertium/cpu.h>
#include <tertium/std.h>

enum {
	LFLAG = 1 << 0,
	SFLAG = 1 << 1,
};

static int
getbyte(ctype_ioq *p, char *s, uint opts)
{
	int r;
	char ch;

	if ((r = c_ioq_get(p, &ch, 1)) < 0) {
		if (!(opts & SFLAG))
			c_err_warn("c_ioq_getall %s", s);
		c_std_exit(2);
	}

	return r ? ch : -1;
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
	int ch[2];
	int i, rval;
	uint opts;
	char bufs[C_BIOSIZ][2];

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

	for (i = 0; i < 2; ++i) {
		if (C_ISDASH(argv[i])) {
			fds[i] = C_FD0;
			argv[i] = "<stdin>";
		} else if ((fds[i] = c_sys_open(argv[i], C_OREAD, 0)) < 0) {
			if (!(opts & SFLAG))
				c_err_warn("c_sys_open %s", argv[i]);
			c_std_exit(2);
		}
		c_ioq_init(&iqs[i], fds[i], bufs[i],
		    sizeof(bufs[i]), &c_sys_read);
	}

	rval = 0;
	bno = line = 1;
	for (;;) {
		ch[0] = getbyte(&iqs[0], argv[0], opts);
		ch[1] = getbyte(&iqs[1], argv[1], opts);
		if (ch[0] == ch[1]) {
			if (ch[0] < 0)
				break;
			if (ch[0] == '\n')
				++line;
		} else if (ch[0] < 0 || ch[1] < 0) {
			if (!(opts & SFLAG))
				c_err_warnx("EOF on %s", argv[ch[1] < 0]);
			rval = 1;
			break;
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

	c_ioq_flush(ioq1);
	return rval;
}
