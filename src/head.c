#include <tertium/cpu.h>
#include <tertium/std.h>

#include "common.h"

ctype_status
head(ctype_arr *p, char *s, usize n)
{
	ctype_ioq ioq;
	ctype_fd fd;
	char buf[C_IOQ_BSIZ];

	if (C_STD_ISDASH(s)) {
		fd = C_IOQ_FD0;
	} else if ((fd = c_nix_fdopen2(s, C_NIX_OREAD)) < 0) {
		c_err_warn("failed to open \"%s\"", s);
		return 1;
	}

	c_ioq_init(&ioq, fd, buf, sizeof(buf), c_nix_fdread);
	while (n--) {
		c_arr_trunc(p, 0, sizeof(uchar));
		switch (c_ioq_getln(p, &ioq)) {
		case -1:
			c_err_die(1, "failed to read \"%s\"", s);
		case 0:
			return 0;
		}
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
	--argc, ++argv;

	n = 10;

	while (c_std_getopt(argmain, argc, argv, "n:")) {
		switch (argmain->opt) {
		case 'n':
			n = estrtovl(argmain->arg, 0, 0, -1);
			break;
		default:
			usage();
		}
	}
	argc -= argmain->idx;
	argv += argmain->idx;

	if (!n) return 0;
	c_mem_set(&arr, sizeof(arr), 0);
	switch (argc) {
	case 0:
		r = head(&arr, "-", n);
		break;
	case 1:
		r = head(&arr, *argv, n);
		++argv;
		break;
	default:
		c_ioq_fmt(ioq1, "==> %s <==\n", *argv);
		r = head(&arr, *argv, n);
		++argv;
	}
	for (; *argv; ++argv) {
		c_ioq_fmt(ioq1, "\n==> %s <==\n", *argv);
		r |= head(&arr, *argv, n);
	}
	c_dyn_free(&arr);
	c_ioq_flush(ioq1);
	return r;
}
