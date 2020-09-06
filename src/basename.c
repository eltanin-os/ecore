#include <tertium/cpu.h>
#include <tertium/std.h>

static void
usage(void)
{
	c_ioq_fmt(ioq2, "usage: %s string [suffix]\n", c_std_getprogname());
	c_std_exit(1);
}

ctype_status
main(int argc, char **argv)
{
	char *s;
	usize n;

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

	--argc;
	if ((uint)argc > 1)
		usage();

	s = c_gen_basename(*argv);
	if (argc) {
		argv++;
		n = c_str_len(s, C_USIZEMAX) - c_str_len(*argv, C_USIZEMAX);
		if (!c_str_cmp(s + n, C_USIZEMAX, *argv))
			s[n] = 0;
	}
	c_ioq_fmt(ioq1, "%s\n", s);
	c_ioq_flush(ioq1);
	return 0;
}
