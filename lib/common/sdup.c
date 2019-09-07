#include <tertium/cpu.h>
#include <tertium/std.h>

#include "common.h"

char *
sdup(char *s)
{
	static char buf[C_PATHMAX];

	return c_mem_cpy(buf, c_str_len(s, C_PATHMAX), s);
}
