#include <tertium/cpu.h>
#include <tertium/std.h>

#include "common.h"

static void
usage(void)
{
	c_ioq_fmt(ioq2, "usage: %s [-f] [-c number | -n number] [file]\n",
	    c_std_getprogname());
	c_std_exit(1);
}

static void
tailbytes(ctype_fd fd, char *fname, usize cnt)
{
	ctype_stat st;
	ctype_fssize siz;

	if (c_sys_fstat(&st, fd) < 0)
		c_err_die(1, "c_sys_fstat %s", fname);

	siz = st.size - cnt;
	if (c_sys_seek(fd, siz, SEEK_SET) < 0)
		c_err_die(1, "c_sys_seek %s", fname);

	if (c_ioq_putfd(ioq1, fd, 0) < 0)
		c_err_die(1, "c_ioq_putfd %s", fname);
}

static void
tail(ctype_fd fd, char *fname, usize cnt)
{
	ctype_arr arr;
	ctype_ioq ioq;
	size r;
	usize cur, nl;
	usize *p;
	char buf[C_BIOSIZ];
	char *s;

	c_mem_set(&arr, sizeof(arr), 0);
	if (c_dyn_ready(&arr, cnt + 1, sizeof(cur)) < 0)
		c_err_die(1, "c_dyn_ready");

	c_ioq_init(&ioq, fd, buf, sizeof(buf), &c_sys_read);
	cur = nl = 0;
	for (;;) {
		if ((r = c_ioq_feed(&ioq)) < 0)
			c_err_die(1, "c_ioq_feed %s", fname);
		if (!r)
			break;
		s = c_ioq_peek(&ioq);
		c_ioq_seek(&ioq, r);
		while (r) {
			++cur;
			if (*s == '\n') {
				p = c_arr_get(&arr, nl, sizeof(*p));
				*p = cur;
				if (++nl > cnt)
					nl = 0;
			}
			++s;
			--r;
		}
	}

	p = c_arr_get(&arr, nl, sizeof(*p));
	if (c_sys_seek(fd, *p, SEEK_SET) < 0)
		c_err_die(1, "c_sys_seek %s", fname);

	if (c_ioq_putfd(ioq1, fd, 0) < 0)
		c_err_die(1, "c_ioq_putfd %s", fname);
}

int
main(int argc, char **argv)
{
	ctype_fd fd;
	int fflag;
	usize cnt;
	void (*tailfn)(ctype_fd, char *, usize);

	c_std_setprogname(argv[0]);

	cnt = 10;
	fflag = 0;
	tailfn = tail;

	C_ARGBEGIN {
	case 'c':
		tailfn = tailbytes;
		cnt = estrtovl(C_EARGF(usage()), 0, 0, C_USIZEMAX);
		break;
	case 'f':
		fflag = 1;
		break;
	case 'n':
		tailfn = tail;
		cnt = estrtovl(C_EARGF(usage()), 0, 0, C_USIZEMAX);
		break;
	default:
		usage();
	} C_ARGEND

	if (!argc || C_ISDASH(*argv)) {
		fd = C_FD0;
		*argv = "<stdin>";
	} else if ((fd = c_sys_open(*argv, C_OREAD, 0)) < 0) {
		c_err_die(1, "c_sys_open %s", *argv);
	}

	tailfn(fd, *argv, cnt);
	c_ioq_flush(ioq1);

	if (fflag) {
		c_mem_set(c_ioq_arr(ioq1), sizeof(ctype_arr), 0);
		for (;;) {
			if (c_ioq_putfd(ioq1, fd, 0) < 0)
				c_err_die(1, "c_ioq_putfd %s", *argv);
		}
	}

	return 0;
}
