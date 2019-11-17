#include <tertium/cpu.h>
#include <tertium/std.h>

#include "common.h"

char *
pathcat(char *f1, char *f2, int cat)
{
	ctype_arr arr;
	ctype_stat st;

	c_arr_init(&arr, gbuf, GBUFSIZ);
	trim_trailing_slash(f2);
	c_arr_fmt(&arr, "%s", f2);

	if (!cat) {
		if (c_sys_stat(&st, f2) < 0)
			return gbuf;
		if (!C_ISDIR(st.mode))
			return gbuf;
	}

	c_arr_fmt(&arr, "/%s", c_gen_basename(f1));
	return c_arr_data(&arr);
}
