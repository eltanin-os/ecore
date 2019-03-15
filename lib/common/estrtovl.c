#include <tertium/cpu.h>
#include <tertium/std.h>

vlong
estrtovl(char *p, int b, vlong l, vlong h)
{
	vlong rv;
	int   e;
	char *s;

	rv = c_std_strtovl(p, b, l, h, &s, &e);

	if (e < 0)
		c_err_die(1, "c_std_strtovl %s", p);

	return rv;
}
