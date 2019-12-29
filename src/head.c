#include <tertium/cpu.h>
#include <tertium/std.h>

#include "common.h"

ctype_status
head(char *p, usize cnt)
{
	ctype_ioq ioq;
	ctype_fd fd;
	size r;
	char buf[C_BIOSIZ];

	if (C_ISDASH(p)) {
		fd = C_FD0;
	} else if ((fd = c_sys_open(p, C_OREAD, 0)) < 0) {
		c_err_warn("c_sys_open %s", p);
		return 1;
	}

	c_ioq_init(&ioq, fd, buf, sizeof(buf), c_sys_read);

	do {
		if ((r = c_ioq_getln(&ioq, c_ioq_arr(ioq1))) < 0) {
			if (errno != C_ENOMEM)
				c_err_die(1, "c_ioq_getln %s", p);
			c_ioq_flush(ioq1);
			continue;
		}
		--cnt;
	} while(cnt && r);

	return 0;
}

static void
usage(void)
{
	c_ioq_fmt(ioq2, "usage: %s [-n number] [file ...]\n",
	    c_std_getprogname());
	c_std_exit(1);
}

int
main(int argc, char **argv)
{
	usize hn;
	int r;

	c_std_setprogname(argv[0]);
	hn = 10;

	C_ARGBEGIN {
	case 'n':
		hn = estrtovl(C_EARGF(usage()), 0, 0, C_USIZEMAX);
		break;
	default:
		usage();
	} C_ARGEND

	if (!hn)
		return 0;

	switch (argc) {
	case 0:
		r = head("-", hn);
		break;
	case 1:
		r = head(*argv, hn);
		--argc, ++argv;
		break;
	default:
		c_ioq_fmt(ioq1, "==> %s <==\n", *argv);
		r = head(*argv, hn);
		--argc, ++argv;
	}

	for (; *argv; --argc, ++argv) {
		c_ioq_fmt(ioq1, "\n==> %s <==\n", *argv);
		r |= head(*argv, hn);
	}

	c_ioq_flush(ioq1);
	return r;
}
