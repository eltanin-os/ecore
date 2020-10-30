#include <tertium/cpu.h>
#include <tertium/std.h>

#include "common.h"

#define HORT(a, b, c) (*(a) == '+') ? (++a, b) : (a += (*(a) == '-'), c)

static ctype_arr arr;

static void
usage(void)
{
	c_ioq_fmt(ioq2, "usage: %s [-f] [-c number | -n number] [file]\n",
	    c_std_getprogname());
	c_std_exit(1);
}

static void
headb(ctype_ioq *p, usize cnt)
{
	size n;

	while (cnt) {
		if ((n = c_ioq_feed(p)) <= 0)
			break;
		if (cnt > (usize)n) {
			c_ioq_seek(p, n);
			cnt -= n;
		} else {
			c_ioq_seek(p, cnt);
			break;
		}
	}
	n = c_ioq_feed(p);
	c_std_allrw(&c_sys_write, C_FD1, c_ioq_peek(p), n);
	c_std_fdcat(C_FD1, c_ioq_fileno(p));
}

static void
head(ctype_ioq *p, usize cnt)
{
	for (; cnt && c_ioq_getln(p, &arr); --cnt)
		c_arr_trunc(&arr, 0, sizeof(uchar));
	c_ioq_feed(p);
	c_std_allrw(&c_sys_write, C_FD1, c_ioq_peek(p), c_ioq_feed(p));
	c_std_fdcat(C_FD1, c_ioq_fileno(p));
}

static void
tailb(ctype_ioq *p, usize cnt)
{
	size len, n;
	char *d, *s;

	++cnt;
	if (c_dyn_ready(&arr, cnt, sizeof(uchar)) < 0)
		c_err_die(1, "c_dyn_ready");
	while ((len = c_ioq_feed(p)) > 0) {
		s = c_ioq_peek(p);
		if (len >= (size)cnt) {
			s += len - cnt;
			c_arr_trunc(&arr, 0, sizeof(uchar));
			c_arr_cat(&arr, s, cnt, sizeof(uchar));
		} else {
			if ((n = c_arr_bytes(&arr)) > len) {
				d = c_arr_data(&arr);
				c_mem_cpy(d, n - len, d + len);
				c_mem_cpy(d + (n - len), len, s);
			} else {
				c_arr_trunc(&arr, 0, sizeof(uchar));
				c_arr_cat(&arr, s, len, sizeof(uchar));
			}
		}
		c_ioq_seek(p, len);
	}
	c_std_allrw(&c_sys_write, C_FD1, c_arr_data(&arr), c_arr_bytes(&arr));
}

static void
tail(ctype_ioq *p, usize cnt)
{
	usize cur, n;
	char *left, *s;

	cur = 0;
	while (c_ioq_getln(p, &arr) > 0) {
		if (cur > cnt) {
			s = c_arr_data(&arr);
			n = c_arr_bytes(&arr);
			left = (char *)c_mem_chr(s, n, '\n') + 1;
			n -= left - s;
			c_mem_cpy(s, n, left);
			c_arr_trunc(&arr, n, sizeof(uchar));
		} else {
			++cur;
		}
	}
	c_std_allrw(&c_sys_write, C_FD1, c_arr_data(&arr), c_arr_bytes(&arr));
}

ctype_status
main(int argc, char **argv)
{
	ctype_ioq ioq;
	usize cnt;
	ctype_fd fd;
	int fflag;
	char *s;
	char buf[C_BIOSIZ];
	void (*tailfn)(ctype_ioq *, usize);

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
			cnt = estrtovl(s, 0, 1, C_USIZEMAX) - 1;
			break;
		case 'f':
			fflag = 1;
			break;
		case 'n':
			s = argmain->arg;
			tailfn = HORT(s, head, tail);
			cnt = estrtovl(s, 0, 1, C_USIZEMAX) - 1;
			break;
		default:
			usage();
		}
	}
	argc -= argmain->idx;
	argv += argmain->idx;

	if (!argc || C_ISDASH(*argv)) {
		c_ioq_init(&ioq, C_FD0, buf, sizeof(buf), &c_sys_read);
		tailfn(&ioq, cnt);
	} else {
		if ((fd = c_sys_open(*argv, C_OREAD, 0)) < 0)
			c_err_die(1, "c_sys_open %s", *argv);
		c_ioq_init(&ioq, fd, buf, sizeof(buf), &c_sys_read);
		tailfn(&ioq, cnt);
		c_dyn_free(&arr);
		if (fflag) {
			for (;;) {
				c_std_fdcat(C_FD1, fd);
				deepsleep(1);
			}
		}
	}
	return 0;
}
