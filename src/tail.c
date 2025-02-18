#include <tertium/cpu.h>
#include <tertium/std.h>

#include "common.h"

#define HEAD_OR_TAIL(a, b) \
((*(a) == '+') ? (++a, ((b) | HEAD)) : (a += (*(a) == '-'), ((b) & ~HEAD)))

enum {
	DEFAULT_LINES_AMOUNT = 10,
	FFLAG = 1 << 0,
	BYTES = 1 << 2,
	HEAD  = 1 << 3,
};

static void
usage(void)
{
	c_ioq_fmt(ioq2, "usage: %s [-f] [-c number | -n number] [file]\n",
	    c_std_getprogname());
	c_std_exit(1);
}

static void
start(ctype_ioq *p, usize cnt, uint opts)
{
	size r;
	char *nl, *s;
	while (cnt) {
		if ((r = c_ioq_feed(p)) <= 0) break;
		if (opts & BYTES) {
			cnt -= r = C_STD_MIN(cnt, (usize)r);
		} else {
			s = c_ioq_peek(p);
			if ((nl = c_mem_chr(s, r, '\n'))) {
				r = (nl - s) + 1;
				--cnt;
			}
		}
		c_ioq_seek(p, r);
	}
	c_nix_allrw(&c_nix_fdwrite, C_IOQ_FD1, c_ioq_peek(p), c_ioq_feed(p));
	c_nix_fdputfd(C_IOQ_FD1, c_ioq_fileno(p), 0);
}

static void
end(ctype_ioq *p, usize cnt, uint opts)
{
	ctype_arr arr;
	usize cur, n;
	size r;
	char *nl, *s;

	cur = 0;
	c_mem_set(&arr, sizeof(arr), 0);

	while ((r = c_ioq_getln(&arr, p)) > 0) {
		s = c_arr_data(&arr);
		n = c_arr_bytes(&arr);
		if (opts & BYTES) {
			if (n > cnt) {
				n -= cur = (n - cnt) - 1;
				c_mem_cpy(s, s + cur, cnt + 1);
				c_arr_trunc(&arr, n , sizeof(uchar));
			}
		} else {
			if (cur > cnt) {
				nl = (char *)c_mem_chr(s, n, '\n') + 1;
				n -= nl - s;
				c_mem_cpy(s, nl, n);
				c_arr_trunc(&arr, n, sizeof(uchar));
			} else {
				++cur;
			}
		}
	}
	c_nix_allrw(&c_nix_fdwrite, C_IOQ_FD1, s, n);
	c_dyn_free(&arr);
}

static ctype_status
tail(int argc, char *s, usize cnt, uint opts)
{
	ctype_stat st;
	ctype_ioq ioq;
	ctype_fd fd;
	char buf[C_IOQ_BSIZ];
	void (*func)(ctype_ioq *, usize, uint);

	func = (opts & HEAD) ? &start : &end;

	if (!argc || C_STD_ISDASH(s)) {
		c_ioq_init(&ioq, C_IOQ_FD0, buf, sizeof(buf), &c_nix_fdread);
		func(&ioq, cnt, opts);
	} else {
		fd = c_nix_fdopen2(s, C_NIX_OREAD);
		if (fd < 0) c_err_die(1, "failed to open \"%s\"", s);
		c_ioq_init(&ioq, fd, buf, sizeof(buf), &c_nix_fdread);
		func(&ioq, cnt, opts);
		if (c_nix_fdstat(&st, fd) < 0) {
			c_err_die(1, "failed to obtain file info \"%s\"", s);
		}
		if (!(C_NIX_ISFIFO(st.mode) || C_NIX_ISREG(st.mode))) {
			opts &= ~FFLAG;
		}
		if (opts & FFLAG) {
			for (;;) {
				c_nix_fdputfd(C_IOQ_FD1, fd, 0);
				deepsleep(1);
			}
		}
		c_nix_fdclose(fd);
	}
	return 0;
}

ctype_status
main(int argc, char **argv)
{
	usize cnt;
	uint opts;
	char *s;

	c_std_setprogname(argv[0]);
	--argc, ++argv;

	cnt = DEFAULT_LINES_AMOUNT;
	opts = 0;

	while (c_std_getopt(argmain, argc, argv, "c:fn:")) {
		s = argmain->arg;
		switch (argmain->opt) {
		case 'c':
			opts = HEAD_OR_TAIL(s, opts) | BYTES;
			cnt = estrtovl(s, 0, 1, -1) - 1;
			break;
		case 'f':
			opts |= FFLAG;
			break;
		case 'n':
			opts = HEAD_OR_TAIL(s, opts) & ~BYTES;
			cnt = estrtovl(s, 0, 1, -1) - 1;
			break;
		default:
			usage();
		}
	}
	argc -= argmain->idx;
	argv += argmain->idx;
	if (argc > 1) usage();
	return tail(argc, *argv, cnt, opts);
}
