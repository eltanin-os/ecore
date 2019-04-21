#include <tertium/cpu.h>
#include <tertium/std.h>

#include "common.h"

static char sbuf[C_PATHMAX];

static int
isdir(char *path)
{
	CStat st;
	return ((!c_sys_stat(&st, path) && C_ISDIR(st.st_mode)));
}

char *
pathcatf(char *buf, usize n, char *f1, char *f2, int concat)
{
	CArr  arr;
	usize r;
	char *sep;

	if (!buf) {
		buf = sbuf;
		n   = sizeof(sbuf);
	}

	c_arr_init(&arr, buf, n);
	r = c_arr_fmt(&arr, "%s", f2);

	if (concat == -1)
		concat = isdir(f2);
	if (concat ==  0)
		return buf;

	sep = buf[r-1] == '/' ? "" : "/";
	c_arr_fmt(&arr, "%s%s", sep, f1);

	return buf;
}
