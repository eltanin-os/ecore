#include <tertium/cpu.h>
#include <tertium/std.h>

static void
usage(void)
{
	c_ioq_fmt(ioq2, "usage: %s [-u] [file ...]\n", c_std_getprogname());
	c_ioq_flush(ioq2);
	c_std_exit(1);
}

int
main(int argc, char **argv)
{
	c_std_setprogname(argv[0]);

	C_ARGBEGIN {
	case 'u':
		ioq1->mb->a = 0;
		break;
	default:
		usage();
	} C_ARGEND

	if (!argc)
		return c_ioq_putfd(ioq1, C_FD0, 0);

	for (; *argv; argc--, argv++) {
		if (C_ISDASH(*argv))
			c_ioq_putfd(ioq1, C_FD0, 0);
		else
			c_ioq_putfile(ioq1, *argv);
	}

	c_ioq_flush(ioq1);

	return 0;
}
