#include <tertium/cpu.h>
#include <tertium/std.h>

#include "common.h"

char *
pathcat(char *f1, char *f2, int cat)
{
	static char buf[C_PATHMAX];
	ctype_arr arr;
	ctype_stat st;
	usize r;
	char *sep;

	c_arr_init(&arr, buf, sizeof(buf));
	r = c_arr_fmt(&arr, "%s", f2);

	if (!cat) {
		if (c_sys_stat(&st, f2) < 0)
			return buf;
		if (!C_ISDIR(st.mode))
			return buf;
	}

	sep = buf[r - 1] == '/' ? "" : "/";
	c_arr_fmt(&arr, "%s%s", sep, c_gen_basename(sdup(f1)));

	return buf;
}
