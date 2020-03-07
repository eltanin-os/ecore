#include <tertium/cpu.h>
#include <tertium/std.h>

#include "common.h"

int
yesno(char *s)
{
	int x;
	char ch;

	x = 1;
	if (c_sys_read(C_FD0, &ch, 1) == 1) {
		x = ((ch | 32) == 'y') ? 0 : 1;
		while (ch != '\n' && c_sys_read(C_FD0, &ch, 1) == 1) ;
	}
	return x;
}
