#include <tertium/cpu.h>
#include <tertium/std.h>

#include "common.h"

static void
usage(void)
{
	c_ioq_fmt(ioq2, "usage: %s [-u] [file ...]\n", c_std_getprogname());
	c_std_exit(1);
}

int
main(int argc, char **argv)
{
	int rv;

	c_std_setprogname(argv[0]);

	C_ARGBEGIN {
	case 'u':
		ioq1->mb->a = 0;
		break;
	default:
		usage();
	} C_ARGEND

	rv = 0;

	if (!argc) {
		argv[0] = "-";
		argv[1] = nil;
	}

	for (; *argv; argc--, argv++) {
		if (C_ISDASH(*argv))
			*argv = "<stdin>";
		if (c_ioq_putfile(ioq1, *argv) < 0)
			rv = c_err_warn("putfile %s", *argv);
	}

	c_ioq_flush(ioq1);

	return rv;
}
