#include <tertium/cpu.h>
#include <tertium/std.h>

#include "common.h"

char *
serr(int e)
{
	static char buf[C_ERRSIZ];

	return c_sys_strerror(e, buf, sizeof(buf));
}
