#include <tertium/cpu.h>
#include <tertium/std.h>

#include "common.h"

static char buf[C_ERRSIZ];

char *
serr(ctype_error e)
{
	return c_std_strerror(e, buf, sizeof(buf));
}
