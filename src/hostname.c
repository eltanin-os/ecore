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

	if (c_std_noopt(argmain, *argv)) usage();
	argc -= argmain->idx;
	argv += argmain->idx;
	if (argc > 1) usage();

	if (argc) {
		if (c_sys_sethostname(*argv, c_str_len(*argv, -1)) < 0) {
			c_err_die(1, "failed to set hostname as \"%s\"", *argv);
		}
	} else {
		if (c_nix_uname(&uts) < 0) {
			c_err_die(1, "failed to obtain sys info");
		}
		c_ioq_fmt(ioq1, "%s\n", uts.nodename);
		c_ioq_flush(ioq1);
	}
	return 0;
}
