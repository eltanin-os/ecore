#include <tertium/cpu.h>
#include <tertium/std.h>

static void
usage(void)
{
	c_ioq_fmt(ioq2, "usage: %s [name]\n", c_std_getprogname());
	c_std_exit(1);
}

ctype_status
main(int argc, char **argv)
{
	ctype_utsname uts;

	c_std_setprogname(argv[0]);
	--argc, ++argv;

	while (c_std_getopt(argmain, argc, argv, "")) {
		switch (argmain->opt) {
		default:
			usage();
		}
	}
	argc -= argmain->idx;
	argv += argmain->idx;

	if (argc > 1)
		usage();

	if (argc) {
		if (c_sys_sethostname(*argv, c_str_len(*argv, C_USIZEMAX)) < 0)
			c_err_die(1, "c_sys_sethostname %s", *argv);
	} else {
		if (c_sys_uname(&uts) < 0)
			c_err_die(1, "c_sys_uname");

		c_ioq_fmt(ioq1, "%s\n", uts.nodename);
		c_ioq_flush(ioq1);
	}
	return 0;
}
