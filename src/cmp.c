#include <tertium/cpu.h>
#include <tertium/std.h>

enum {
	LFLAG = 1 << 0,
	SFLAG = 1 << 1,
};

struct file {
	ctype_ioq ioq;
	ctype_fd fd;
	int ch;
	char buf[C_BIOSIZ];
};

static int
getbyte(ctype_ioq *p, char *s, uint opts)
{
	ctype_status r;
	char ch;

	if ((r = c_ioq_get(p, &ch, sizeof(ch))) < 0) {
		if (!(opts & SFLAG))
			c_err_warn("c_ioq_get %s", s);
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
	struct file sf[2];
	ctype_status r;
	usize line, bno;
	int i;
	uint opts;

	c_std_setprogname(argv[0]);
	--argc, ++argv;

	opts = 0;

	while (c_std_getopt(argmain, argc, argv, "ls")) {
		switch (argmain->opt) {
		case 'l':
			opts = LFLAG;
			break;
		case 's':
			opts = SFLAG;
			break;
		default:
			usage();
		}
	}
	argc -= argmain->idx;
	argv += argmain->idx;

	if (argc - 2)
		usage();

	for (i = 0; i < 2; ++i) {
		if (C_ISDASH(argv[i])) {
			sf[i].fd = C_FD0;
			argv[i] = "<stdin>";
		} else if ((sf[i].fd = c_sys_open(argv[i], C_OREAD, 0)) < 0) {
			if (!(opts & SFLAG))
				c_err_warn("c_sys_open %s", argv[i]);
			c_std_exit(2);
		}
		c_ioq_init(&sf[i].ioq, sf[i].fd,
		    sf[i].buf, sizeof(sf[i].buf), &c_sys_read);
	}

	r = 0;
	bno = line = 1;
	for (;;) {
		sf[0].ch = getbyte(&sf[0].ioq, argv[0], opts);
		sf[1].ch = getbyte(&sf[1].ioq, argv[1], opts);
		if (sf[0].ch == sf[1].ch) {
			if (sf[0].ch < 0)
				break;
			if (sf[0].ch == '\n')
				++line;
		} else if (sf[0].ch < 0 || sf[1].ch < 0) {
			if (!(opts & SFLAG))
				c_err_warnx("EOF on %s", argv[sf[1].ch < 0]);
			r = 1;
			break;
		} else if (!(opts & LFLAG)) {
			if (!(opts & SFLAG))
				c_ioq_fmt(ioq1,
				    "%s %s differ: char %ud, line %ud\n",
				    argv[0], argv[1], bno, line);
			r = 1;
			break;
		} else {
			c_ioq_fmt(ioq1, "%ud %o %o\n", bno, sf[0].ch, sf[1].ch);
			r = 1;
		}
		++bno;
	}
	c_ioq_flush(ioq1);
	return r;
}
