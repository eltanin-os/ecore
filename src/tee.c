#include <tertium/cpu.h>
#include <tertium/std.h>

static void
usage(void)
{
	c_ioq_fmt(ioq2, "usage: %s [-ai] [file ...]\n", c_std_getprogname());
	c_std_exit(1);
}

int
main(int argc, char **argv)
{
	ctype_fd *fds;
	size r;
	uint opts;
	int i;
	char buf[C_BIOSIZ];

	c_std_setprogname(argv[0]);

	opts = C_OWRITE | C_OTRUNC | C_OCREATE;

	C_ARGBEGIN {
	case 'a':
		opts = (opts & ~C_OTRUNC) | C_OAPPEND;
		break;
	case 'i':
		break;
	default:
		usage();
	} C_ARGEND

	if (!(fds = c_std_alloc(argc + 1, sizeof(*fds))))
		c_err_die(1, "c_std_alloc");

	for (i = 0; i < argc; ++i)
		if ((fds[i] = c_sys_open(argv[i], opts, C_DEFFILEMODE)) < 0)
			c_err_die(1, "c_sys_open %s", argv[i]);
	fds[i] = C_FD1;
	++argc;
	while ((r = c_sys_read(C_FD0, buf, sizeof(buf))) > 0)
		for (i = 0; i < argc; ++i)
			if (c_std_allrw(&c_sys_write, fds[i], buf, r) < 0)
				c_err_die(1, "c_sys_write %s",
				    argv[i] ? argv[i] : "<stdout>");
	if (r < 0)
		c_err_die(1, "c_sys_read");

	return 0;
}
