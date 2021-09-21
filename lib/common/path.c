#include <tertium/cpu.h>
#include <tertium/std.h>

#include "common.h"

char *
pathcat(char *s, char *d, int cat)
{
	static char buf[C_PATHMAX];
	ctype_arr arr;
	ctype_stat st;

	c_str_rtrim(d, -1, "/");
	c_arr_init(&arr, buf, sizeof(buf));
	c_arr_fmt(&arr, "%s", d);
	if (!cat) {
		if (c_nix_stat(&st, d) < 0 || !C_ISDIR(st.mode))
			return c_arr_data(&arr);
	}
	c_arr_fmt(&arr, "/%s", c_gen_basename(s));
	return c_arr_data(&arr);
}
