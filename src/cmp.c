#include <tertium/cpu.h>
#include <tertium/std.h>

enum {
	LFLAG = 1 << 0,
	SFLAG = 1 << 1,
};

static int
getbyte(ctype_ioq *p, char *s, uint opts)
{
	ctype_status r;
	char buf;
	if ((r = c_ioq_get(&buf, sizeof(buf), p)) < 0) {
		if (!(opts & SFLAG)) c_err_warn("failed to read \"%s\"", s);
		c_std_exit(2);
	}
	if (!r) return -1;
	return buf;
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
	ctype_ioq file[2];
	usize line, bno;
	ctype_fd fd;
	ctype_status r;
	int ch[2], i;
	uint opts;
	char buf[2][C_IOQ_SMALLBSIZ];

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
	if (argc - 2) usage();

	for (i = 0; i < 2; ++i) {
		if (C_STD_ISDASH(argv[i])) {
			fd = C_IOQ_FD0;
			argv[i] = "<stdin>";
		} else if ((fd = c_nix_fdopen2(argv[i], C_NIX_OREAD)) < 0) {
			if (!(opts & SFLAG)) {
				c_err_warn("failed to open \"%s\"", argv[i]);
			}
			c_std_exit(2);
		}
		c_ioq_init(&file[i], fd, buf[i], sizeof(buf[i]), &c_nix_fdread);
	}
	r = 0;
	bno = line = 1;
	for (;;) {
		ch[0] = getbyte(&file[0], argv[0], opts);
		ch[1] = getbyte(&file[1], argv[1], opts);
		if (ch[0] == ch[1]) {
			if (ch[0] < 0) break;
			if (ch[0] == '\n') ++line;
		} else if (ch[0] < 0 || ch[1] < 0) {
			if (!(opts & SFLAG)) {
				c_err_warnx("EOF on %s", argv[ch[1] < 0]);
			}
			r = 1;
			break;
		} else if (!(opts & LFLAG)) {
			if (!(opts & SFLAG)) {
				c_ioq_fmt(ioq1,
				    "%s %s differ: char %ud, line %ud\n",
				    argv[0], argv[1], bno, line);
			}
			r = 1;
			break;
		} else {
			c_ioq_fmt(ioq1, "%ud %o %o\n", bno, ch[0], ch[1]);
			r = 1;
		}
		++bno;
	}
	c_ioq_flush(ioq1);
	return r;
}
