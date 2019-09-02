#include <tertium/cpu.h>
#include <tertium/std.h>

static void
usage(void)
{
	c_ioq_fmt(ioq2, "usage: %s [expletive]\n", c_std_getprogname());
	c_std_exit(1);
}

int
main(int argc, char **argv)
{
	CArr  arr;
	usize n;
	char *s;
	char  buf[C_BIOSIZ];

	c_std_setprogname(argv[0]);

	C_ARGBEGIN {
	default:
		usage();
	} C_ARGEND

	s = argc ? *argv : "y";
	c_arr_init(&arr, buf, sizeof(buf));
	while (c_arr_fmt(&arr, "%s\n", s) > 0)
		;

	s = c_arr_data(&arr);
	n = c_arr_bytes(&arr);
	for (;;)
		c_sys_allrw(c_sys_write, C_FD1, s, n);

	return 1;
}
