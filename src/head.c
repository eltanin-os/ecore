#include <tertium/cpu.h>
#include <tertium/std.h>

#include "common.h"

static int hn = 10;

static int
head(char *p)
{
	ctype_arr arr;
	ctype_ioq ioq;
	ctype_fd fd;
	int i;
	char buf[C_BIOSIZ];

	if (C_ISDASH(p)) {
		fd = 0;
	} else if ((fd = c_sys_open(p, C_OREAD, 0)) < 0) {
		c_err_warn("c_sys_open %s", p);
		return 1;
	}

	c_arr_init(&arr, buf, sizeof(buf));
	c_ioq_init(&ioq, fd, &arr, c_sys_read);

	for (i = 0; i < hn; i++) {
		if (c_ioq_getln(&ioq, ioq1->mb) < 0)
			break;
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

int
main(int argc, char **argv)
{
	int r;

	c_std_setprogname(argv[0]);

	C_ARGBEGIN {
	case 'n':
		hn = estrtovl(C_EARGF(usage()), 0, 0, C_INTMAX);
		break;
	default:
		usage();
	} C_ARGEND

	switch (argc) {
	case 0:
		r = head("-");
		break;
	case 1:
		r = head(*argv);
		argc--, argv++;
		break;
	default:
		c_ioq_fmt(ioq1, "==> %s <==\n", *argv);
		r = head(*argv);
		argc--, argv++;
	}

	for (; *argv; argc--, argv++) {
		c_ioq_fmt(ioq1, "\n==> %s <==\n", *argv);
		r |= head(*argv);
	}

	c_ioq_flush(ioq1);

	return r;
}
