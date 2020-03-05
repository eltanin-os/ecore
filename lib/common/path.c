#include <tertium/cpu.h>
#include <tertium/std.h>

#include "common.h"

void
trim_trailing_slash(char *s)
{
	usize n;

	n = c_str_len(s, C_USIZEMAX);
	for (;;) {
		if (!(--n))
			break;
		if (s[n] != '/')
			break;
		s[n] = 0;
	}
}
