#include <tertium/cpu.h>
#include <tertium/std.h>

static void
usage(void)
{
	c_ioq_fmt(ioq2, "usage: %s [expletive]", c_std_getprogname());
	c_ioq_flush(ioq2);
	c_std_exit(1);
}
int
main(int argc, char **argv)
{
	CArr  arr;
	usize n;
	char  buf[C_BIOSIZ];
	char *s;

	c_std_setprogname(argv[0]);

	C_ARGBEGIN {
	default:
		usage();
	} C_ARGEND

	s = argc ? *argv : "y";
	c_arr_init(&arr, buf, sizeof(buf));

	while (c_arr_fmt(&arr, "%s\n", s) > 0)
		;

	s = c_arr_get(&arr, 0, sizeof(uchar));
	n = c_arr_len(&arr, sizeof(uchar));
	for (;;)
		c_sys_allrw(c_sys_write, C_FD1, s, n);

	return 1;
}
