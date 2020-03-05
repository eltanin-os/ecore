#include <tertium/cpu.h>
#include <tertium/std.h>

#include "common.h"

vlong
estrtovl(char *p, int b, vlong l, vlong h)
{
	ctype_status e;
	vlong r;

	r = c_std_strtovl(p, b, l, h, nil, &e);
	if (e < 0)
		c_err_die(1, "c_std_strtovl %s", p);

	return r;
}
