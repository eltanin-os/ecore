#include <tertium/cpu.h>
#include <tertium/std.h>

#include "common.h"

static char *av[2];

char **
tmpargv(char *s)
{
	av[0] = s;
	return av;
}
