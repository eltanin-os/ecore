#include <tertium/cpu.h>
#include <tertium/std.h>

#include "common.h"

ctype_status
head(ctype_arr *p, char *s, usize n)
{
	ctype_ioq ioq;
	ctype_fd fd;
	size r;
	char buf[C_BIOSIZ];

	if (C_ISDASH(s)) {
		fd = C_FD0;
	} else if ((fd = c_sys_open(s, C_OREAD, 0)) < 0) {
		c_err_warn("c_sys_open %s", s);
		return 1;
	}

	c_ioq_init(&ioq, fd, buf, sizeof(buf), c_sys_read);
	while (n--) {
		c_arr_trunc(p, 0, sizeof(uchar));
		if ((r = c_ioq_getln(&ioq, p)) < 0)
			c_err_die(1, "c_ioq_getln %s", s);
		if (!r)
			break;
		c_ioq_fmt(ioq1, "%s", c_arr_data(p));
	}
	return 0;
}

static void
usage(void)
{
	c_ioq_fmt(ioq2, "usage: %s [-n number] [file ...]\n",
	    c_std_getprogname());
	c_std_exit(1);
}

ctype_status
main(int argc, char **argv)
{
	ctype_arr arr;
	ctype_status r;
	usize n;

	c_std_setprogname(argv[0]);
	n = 10;

	C_ARGBEGIN {
	case 'n':
		n = estrtovl(C_EARGF(usage()), 0, 0, C_USIZEMAX);
		break;
	default:
		usage();
	} C_ARGEND

	if (!n)
		return 0;

	c_mem_set(&arr, sizeof(arr), 0);
	switch (argc) {
	case 0:
		r = head(&arr, "-", n);
		break;
	case 1:
		r = head(&arr, *argv, n);
		--argc, ++argv;
		break;
	default:
		c_ioq_fmt(ioq1, "==> %s <==\n", *argv);
		r = head(&arr, *argv, n);
		--argc, ++argv;
	}
	for (; *argv; --argc, ++argv) {
		c_ioq_fmt(ioq1, "\n==> %s <==\n", *argv);
		r |= head(&arr, *argv, n);
	}
	c_dyn_free(&arr);
	c_ioq_flush(ioq1);
	return r;
}
