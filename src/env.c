#include <tertium/cpu.h>
#include <tertium/std.h>

#include "common.h"

static void
usage(void)
{
	c_ioq_fmt(ioq2, "usage: %s [-i] [utility [args ...]]\n",
	    c_std_getprogname());
	c_ioq_flush(ioq2);
	c_std_exit(1);
}

int
main(int argc, char **argv)
{
	char *s;

	c_std_setprogname(argv[0]);

	C_ARGBEGIN {
	case 'i':
		environ = nil;
		break;
	default:
		usage();
	} C_ARGEND

	if (!argc) {
		for (; *environ; environ++)
			c_ioq_fmt(ioq1, "%s\n", *environ);
		c_ioq_flush(ioq1);
		c_std_exit(0);
	}

	for (; *argv && (s = c_str_chr(*argv, C_USIZEMAX, '=')); argv++)
		c_exc_setenv(*argv, s+1);

	c_exc_run(*argv, argv);
	c_err_die(126 + (errno == C_ENOENT), "c_exc_run %s", *argv);
}
