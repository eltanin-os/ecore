#include <tertium/cpu.h>
#include <tertium/std.h>

static void
usage(void)
{
	c_ioq_fmt(ioq2, "usage: %s [-i] [utility [args ...]]\n",
	    c_std_getprogname());
	c_std_exit(1);
}

ctype_status
main(int argc, char **argv)
{
	char *s;

	c_std_setprogname(argv[0]);
	--argc, ++argv;

	while (c_std_getopt(argmain, argc, argv, "i")) {
		switch (argmain->opt) {
		case 'i':
			*environ = nil;
			break;
		default:
			usage();
		}
	}
	argc -= argmain->idx;
	argv += argmain->idx;

	if (!argc) {
		for (; *environ; ++environ)
			c_ioq_fmt(ioq1, "%s\n", *environ);
		c_std_exit(0);
	}

	for (; *argv; --argc, ++argv) {
		if (!(s = c_str_chr(*argv, C_USIZEMAX, '=')))
			break;
		c_exc_setenv(*argv, s + 1);
	}

	c_exc_run(*argv, argv);
	c_err_die(126 + (errno == C_ENOENT), "c_exc_run %s", *argv);
	/* NOT REACHED */
	return 0;
}
