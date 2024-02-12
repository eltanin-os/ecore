#include <tertium/cpu.h>
#include <tertium/std.h>

static char *
getpwd(void)
{
	ctype_stat pwd, dot;
	char *s;

	if (!(s = c_std_getenv("PWD"))) return nil;
	s = c_nix_normalizepath(s, -1);
	if (s[0] != '/') return nil;

	if (c_str_str(s, -1, "/../")) return nil;

	if (c_nix_stat(&pwd, s) < 0 || c_nix_stat(&dot, ".") < 0) return nil;
	if (pwd.dev == dot.dev && pwd.ino == dot.ino) return s;
	return nil;
}

static void
usage(void)
{
	c_ioq_fmt(ioq2, "usage: %s [-L|-P]\n", c_std_getprogname());
	c_std_exit(1);
}

ctype_status
main(int argc, char **argv)
{
	int mode;
	char *s;
	char buf[C_LIM_PATHMAX];

	c_std_setprogname(argv[0]);
	--argc, ++argv;

	mode = 1;

	while (c_std_getopt(argmain, argc, argv, "LP")) {
		switch (argmain->opt) {
		case 'L':
			mode = 1;
			break;
		case 'P':
			mode = 0;
			break;
		default:
			usage();
		}
	}
	argc -= argmain->idx;
	argv += argmain->idx;
	if (argc) usage();

	switch (mode) {
	case 1:
		if ((s = getpwd())) break;
		/* FALLTHROUGH */
	case 0:
		if (!(s = c_nix_getcwd(buf, sizeof(buf)))) {
			c_err_die(1, "failed to get current dir path");
		}
	}
	c_ioq_fmt(ioq1, "%s\n", s);
	c_ioq_flush(ioq1);
	return 0;
}
