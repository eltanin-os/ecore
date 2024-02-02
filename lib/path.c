#include <tertium/cpu.h>
#include <tertium/std.h>

#include "common.h"

char *
pathcat(char *s, char *d, int cat)
{
	static char buf[C_LIM_PATHMAX];
	ctype_arr arr;
	ctype_stat st;

	c_arr_init(&arr, buf, sizeof(buf));
	c_arr_fmt(&arr, "%s", d);
	/* normalize path and recalculate */
	d = c_arr_data(&arr);
	c_str_rtrim(d, c_arr_bytes(&arr), "/");
	c_nix_normalizepath(d, c_arr_bytes(&arr));
	c_arr_trunc(&arr, c_str_len(d, c_arr_bytes(&arr)), sizeof(uchar));

	if (!cat && (c_nix_stat(&st, d) < 0 || !C_NIX_ISDIR(st.mode))) {
		return d;
	}
	c_arr_fmt(&arr, "/%s", c_gen_basename(s));
	return c_arr_data(&arr);
}
