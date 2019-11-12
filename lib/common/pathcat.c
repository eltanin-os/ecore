#include <tertium/cpu.h>
#include <tertium/std.h>

#include "common.h"

char *
pathcat(char *f1, char *f2, int cat)
{
	ctype_arr arr;
	ctype_stat st;
	usize r;
	char *sep;

	c_arr_init(&arr, gbuf, sizeof(gbuf));
	r = c_arr_fmt(&arr, "%s", f2);

	if (!cat) {
		if (c_sys_stat(&st, f2) < 0)
			return gbuf;
		if (!C_ISDIR(st.mode))
			return gbuf;
	}

	sep = gbuf[r - 1] == '/' ? "" : "/";
	c_arr_fmt(&arr, "%s%s", sep, c_gen_basename(sdup(f1)));

	return gbuf;
}
