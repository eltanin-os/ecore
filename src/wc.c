#include <tertium/cpu.h>
#include <tertium/std.h>

enum {
	CFLAG = 1 << 0,
	MFLAG = 1 << 1,
	LFLAG = 1 << 2,
	WFLAG = 1 << 3,
};

struct total {
	usize nc;
	usize nl;
	usize nw;
};

static void
display(char *fname, usize nc, usize nl, usize nw, uint opts)
{
	if (opts & LFLAG)
		c_ioq_fmt(ioq1, "%llud ", (uvlong)nl);
	if (opts & WFLAG)
		c_ioq_fmt(ioq1, "%llud ", (uvlong)nw);
	if (opts & (CFLAG|MFLAG))
		c_ioq_fmt(ioq1, "%llud ", (uvlong)nc);
	c_ioq_fmt(ioq1, "%s\n", fname);
}

static int
wc(struct total *t, ctype_fd fd, char *fname, uint opts)
{
	ctype_rune rune;
	ctype_ioq ioq;
	size r;
	usize nc, nl, nw;
	int nr;
	char buf[C_BIOSIZ];
	char *p;

	c_ioq_init(&ioq, fd, buf, sizeof(buf), &c_sys_read);
	nc = nl = nw = 0;
	for (;;) {
		if ((r = c_ioq_feed(&ioq)) < 0)
			c_err_die(1, "c_ioq_feed %s", fname);
		if (!r)
			break;
		p = c_ioq_peek(&ioq);
		c_ioq_seek(&ioq, r);
		while (r) {
			nr  = c_utf8_chartorune(&rune, p);
			nc += (opts & CFLAG) ? nr : 1;
			if (!c_chr_isspace(*(p - 1)) && c_chr_isspace(*p))
				++nw;
			if (*p == '\n')
				++nl;
			p += nr;
			r -= nr;
		}
	}

	if (t) {
		t->nc += nc;
		t->nl += nl;
		t->nw += nw;
	}

	display(fname, nc, nl, nw, opts);

	if (fd != C_FD0)
		c_sys_close(fd);

	return 0;
}

static void
usage(void)
{
	c_ioq_fmt(ioq2, "usage: %s [-c|-m] [-lw] [file ...]\n",
	    c_std_getprogname());
	c_std_exit(1);
}

int
main(int argc, char **argv)
{
	struct total total;
	struct total *p;
	ctype_fd fd;
	int rv;
	uint opts;

	c_std_setprogname(argv[0]);

	opts = 0;

	C_ARGBEGIN {
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
	} C_ARGEND

	if (!opts)
		opts = CFLAG|LFLAG|WFLAG;

	if (!argc)
		c_std_exit(wc(nil, C_FD0, "", opts));

	rv = 0;
	c_mem_set(&total, sizeof(total), 0);

	p = argc > 1 ? &total : nil;
	for (; *argv; --argc, ++argv) {
		if (C_ISDASH(*argv)) {
			fd = C_FD0;
			*argv = "<stdin>";
		} else if ((fd = c_sys_open(*argv, C_OREAD, 0)) < 0) {
			rv = c_err_warn("c_sys_open %s", *argv);
			continue;
		}
		rv |= wc(p, fd, *argv, opts);
	}

	if (p)
		display("total", p->nc, p->nl, p->nw, opts);

	c_ioq_flush(ioq1);
	return rv;
}
