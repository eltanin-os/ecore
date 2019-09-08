#include <tertium/cpu.h>
#include <tertium/std.h>

#include "common.h"

char *
sdup(char *s)
{
	static char buf[C_PATHMAX];
	usize n;

	n = c_str_len(s, C_PATHMAX);
	c_mem_cpy(buf, n, s);
	buf[n] = 0;
	return buf;
}
