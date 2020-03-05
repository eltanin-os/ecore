#include <tertium/cpu.h>
#include <tertium/std.h>

#include "common.h"

static char buf[C_PATHMAX];

char *
pathcat(char *f1, char *f2, int cat)
{
	ctype_arr arr;
	ctype_stat st;

	c_arr_init(&arr, buf, sizeof(buf));
	trim_trailing_slash(f2);
	c_arr_fmt(&arr, "%s", f2);
	if (!cat) {
		if (c_sys_stat(f2, &st) < 0 || !C_ISDIR(st.mode))
			return c_arr_data(&arr);
	}
	c_arr_fmt(&arr, "/%s", c_gen_basename(f1));
	return c_arr_data(&arr);
}
