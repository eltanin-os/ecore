#include <tertium/cpu.h>
#include <tertium/std.h>

#include "common.h"

char *
serr(int e)
{
	return c_std_strerror(e, gbuf, C_ERRSIZ);
}
