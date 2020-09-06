#include <tertium/cpu.h>
#include <tertium/std.h>

#include "common.h"

#define HORT(a, b, c) (*(a) == '+') ? (++a, b) : (a += (*(a) == '-'), c)
#define WATCH(a) { if (fflag) for (;;) c_std_fdcat(C_FD1, (a)); }

static void
usage(void)
{
	c_ioq_fmt(ioq2, "usage: %s [-f] [-c number | -n number] [file]\n",
	    c_std_getprogname());
	c_std_exit(1);
}

static void
headb(ctype_fd fd, char *fname, usize cnt)
{
	ctype_stat st;

	if (c_sys_fstat(&st, fd) < 0)
		c_err_die(1, "c_sys_fstat %s", fname);

	if (cnt > st.size)
		return;

	if (c_sys_seek(fd, cnt, C_SEEKSET) < 0)
		c_err_die(1, "c_sys_seek %s", fname);

	if (c_ioq_putfd(ioq1, fd, 0) < 0)
		c_err_die(1, "c_ioq_putfd %s", fname);
}

static void
head(ctype_fd fd, char *fname, usize cnt)
{
	ctype_ioq ioq;
	ctype_fssize siz, lsiz;
	size r;
	char buf[C_BIOSIZ];
	char *s;

	lsiz = siz = 0;
	c_ioq_init(&ioq, fd, buf, sizeof(buf), &c_sys_read);
	while (cnt) {
		if ((r = c_ioq_feed(&ioq)) < 0)
			c_err_die(1, "c_ioq_feed %s", fname);
		if (!r)
			break;
		s = c_ioq_peek(&ioq);
		c_ioq_seek(&ioq, r);
		while (r) {
			++siz;
			if (*s == '\n') {
				if (!--cnt)
					break;
				lsiz = siz;
			}
			--r;
			++s;
		}
	}

	if (c_sys_seek(fd, lsiz, C_SEEKSET) < 0)
		c_err_die(1, "c_sys_seek %s", fname);

	if (c_ioq_putfd(ioq1, fd, 0) < 0)
		c_err_die(1, "c_ioq_putfd %s", fname);
}

static void
tailb(ctype_fd fd, char *fname, usize cnt)
{
	ctype_stat st;
	ctype_fssize siz;

	if (c_sys_fstat(&st, fd) < 0)
		c_err_die(1, "c_sys_fstat %s", fname);

	siz = st.size > cnt ? st.size - cnt : 0;
	if (c_sys_seek(fd, siz, C_SEEKSET) < 0)
		c_err_die(1, "c_sys_seek %s", fname);

	if (c_ioq_putfd(ioq1, fd, 0) < 0)
		c_err_die(1, "c_ioq_putfd %s", fname);
}

static void
tail(ctype_fd fd, char *fname, usize cnt)
{
	ctype_ioq ioq;
	ctype_arr arr;
	ctype_fssize cur;
	size r;
	usize nl;
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
			--r;
			++s;
		}
	}

	p = c_arr_get(&arr, nl, sizeof(*p));
	if (c_sys_seek(fd, *p, C_SEEKSET) < 0)
		c_err_die(1, "c_sys_seek %s", fname);

	if (c_ioq_putfd(ioq1, fd, 0) < 0)
		c_err_die(1, "c_ioq_putfd %s", fname);
}

ctype_status
main(int argc, char **argv)
{
	ctype_fd fd;
	usize cnt;
	int fflag;
	char *s;
	char tmp[18];
	void (*tailfn)(ctype_fd, char *, usize);

	c_std_setprogname(argv[0]);
	--argc, ++argv;

	cnt = 10;
	fflag = 0;
	tailfn = tail;

	while (c_std_getopt(argmain, argc, argv, "c:fn:")) {
		switch (argmain->opt) {
		case 'c':
			s = argmain->arg;
			tailfn = HORT(s, headb, tailb);
			cnt = estrtovl(s, 0, 1, C_USIZEMAX);
		case 'f':
			fflag = 1;
			break;
		case 'n':
			s = argmain->arg;
			tailfn = HORT(s, head, tail);
			cnt = estrtovl(s, 0, 1, C_USIZEMAX);
			break;
		default:
			usage();
		}
	}
	argc -= argmain->idx;
	argv += argmain->idx;

	if (!argc || C_ISDASH(*argv)) {
		*argv = "<stdin>";
		c_mem_cpy(tmp, sizeof(tmp), "/tmp/.tmp.XXXXXXXX");
		if ((fd = c_std_mktemp(tmp, sizeof(tmp), C_OTMPANON)) < 0)
			c_err_die(1, "c_std_mktemp");
		c_std_fdcat(fd, C_FD0);
		c_sys_seek(fd, 0, C_SEEKSET);
		tailfn(fd, *argv, cnt);
	} else {
		if ((fd = c_sys_open(*argv, C_OREAD, 0)) < 0)
			c_err_die(1, "c_sys_open %s", *argv);
		tailfn(fd, *argv, cnt);
		c_ioq_flush(ioq1);
		WATCH(fd);
	}
	c_ioq_flush(ioq1);
	return 0;
}
