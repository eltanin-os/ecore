#include <tertium/cpu.h>
#include <tertium/std.h>

#include "common.h"

char *
sdup(char *s)
{
	usize n;

	n = c_str_len(s, sizeof(gbuf));
	c_mem_cpy(gbuf, n, s);
	gbuf[n] = 0;
	return gbuf;
}
