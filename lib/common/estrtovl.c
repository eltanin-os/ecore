#include <tertium/cpu.h>
#include <tertium/std.h>

vlong
estrtovl(char *p, int b, vlong l, vlong h)
{
	vlong rv;
	int e;

	rv = c_std_strtovl(p, b, l, h, nil, &e);

	if (e < 0)
		c_err_die(1, "c_std_strtovl %s", p);

	return rv;
}
