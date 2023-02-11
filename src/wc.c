#include <tertium/cpu.h>
#include <tertium/std.h>

#define isword(a, b) (!c_utf8_isspace((a)) && c_utf8_isspace((b)))

enum {
	CFLAG = 1 << 0,
	MFLAG = 1 << 1,
	LFLAG = 1 << 2,
	WFLAG = 1 << 3,
};

struct clw {
	usize c;
	usize l;
	usize w;
};

static uint opts;
static struct clw *clw;

static void
display(char *fname, struct clw *p)
{
	if (opts & LFLAG) c_ioq_fmt(ioq1, "%llud ", (uvlong)p->l);
	if (opts & WFLAG) c_ioq_fmt(ioq1, "%llud ", (uvlong)p->w);
	if (opts & (CFLAG|MFLAG)) c_ioq_fmt(ioq1, "%llud ", (uvlong)p->c);
	c_ioq_fmt(ioq1, "%s\n", fname);
}

static void
wc(char *fname, ctype_fd fd)
{
	struct clw n;
	ctype_ioq ioq;
	ctype_rune rune;
	size r;
	int len;
	char buf[C_IOQ_BSIZ];
	char *p;

	c_ioq_init(&ioq, fd, buf, sizeof(buf), &c_nix_fdread);
	rune = 0;
	c_mem_set(&n, sizeof(n), 0);
	for (;;) {
		r = c_ioq_feed(&ioq);
		if (r < 0) c_err_die(1, "failed to read \"%s\"", fname);
		if (!r) break;
		p = c_ioq_peek(&ioq);
		c_ioq_seek(&ioq, r);
		for (;;) {
			if (rune) n.w += isword(rune, *p);
			if (!r) break;
			len = c_utf8_chartorune(&rune, p);
			n.c += (opts & CFLAG) ? len : 1;
			n.l += (*p == '\n');
			p += len;
			r -= len;
		}
	}
	if (fd != C_IOQ_FD0) c_nix_fdclose(fd);
	display(fname, &n);
	if (clw) {
		clw->c += n.c;
		clw->l += n.l;
		clw->w += n.w;
	}
}

static void
usage(void)
{
	c_ioq_fmt(ioq2, "usage: %s [-c|-m] [-lw] [file ...]\n",
	    c_std_getprogname());
	c_std_exit(1);
}

ctype_status
main(int argc, char **argv)
{
	struct clw total;
	ctype_fd fd;
	ctype_status r;

	c_std_setprogname(argv[0]);
	--argc, ++argv;

	while (c_std_getopt(argmain, argc, argv, "cmlw")) {
		switch (argmain->opt) {
		case 'c':
			opts &= ~MFLAG;
			opts |= CFLAG;
			break;
		case 'm':
			opts &= ~CFLAG;
			opts |= MFLAG;
			break;
		case 'l':
			opts |= LFLAG;
			break;
		case 'w':
			opts |= WFLAG;
			break;
		default:
			usage();
		}
	}
	argc -= argmain->idx;
	argv += argmain->idx;

	if (!opts) opts = CFLAG|LFLAG|WFLAG;

	if (!argc) {
		wc("", C_IOQ_FD0);
		c_std_exit(0);
	} else if (argc > 1) {
		c_mem_set(&total, sizeof(total), 0);
		clw = &total;
	}

	r = 0;
	for (; *argv; ++argv) {
		if (C_STD_ISDASH(*argv)) {
			fd = C_IOQ_FD0;
			*argv = "<stdin>";
		} else if ((fd = c_nix_fdopen2(*argv, C_NIX_OREAD)) < 0) {
			r = c_err_warn("failed to open \"%s\"", *argv);
			continue;
		}
		wc(*argv, fd);
	}
	if (clw) display("total", &total);
	c_ioq_flush(ioq1);
	return r;
}
