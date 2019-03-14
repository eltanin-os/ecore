#include <tertium/cpu.h>
#include <tertium/std.h>

static void
usage(void)
{
	c_ioq_fmt(ioq2, "usage: %s string [suffix]\n", c_std_getprogname());
	c_ioq_flush(ioq2);
	c_std_exit(1);
}

int
main(int argc, char **argv)
{
	char *s;
	usize n;

	c_std_setprogname(argv[0]);

	C_ARGBEGIN {
	default:
		usage();
	} C_ARGEND

	if ((uint)--argc > 1)
		usage();

	s = c_gen_basename(*argv++);
	n = c_str_len(s);

	if (argc) {
		n -= c_str_len(*argv);
		if (!c_str_cmp(s+n, C_USIZEMAX, *argv))
			s[n] = 0;
	}

	c_ioq_fmt(ioq1, "%s\n", s);
	c_ioq_flush(ioq1);

	return 0;
}
