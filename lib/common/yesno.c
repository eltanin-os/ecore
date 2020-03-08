#include <tertium/cpu.h>
#include <tertium/std.h>

#include "common.h"

int
prompt(char *s)
{
	ctype_stat st;

	if (c_sys_stat(s, &st) < 0) {
		if (errno == C_ENOENT)
			return 0;
		return c_err_warn("c_sys_stat %s", s);
	}

	c_ioq_fmt(ioq2, "%s: overwrite %s? ", c_std_getprogname(), s);
	c_ioq_flush(ioq2);
	return yesno(s);
}

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
