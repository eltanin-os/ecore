#include <tertium/cpu.h>
#include <tertium/std.h>

#include "common.h"

char *
sndup(char *s, usize n)
{
	c_mem_cpy(gbuf, n, s);
	gbuf[n] = 0;
	return gbuf;
}
