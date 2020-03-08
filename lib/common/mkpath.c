#include <tertium/cpu.h>
#include <tertium/std.h>

#include "common.h"

static ctype_status
makedir(char *s, uint mode)
{
	ctype_stat st;

	if (c_sys_mkdir(s, mode) < 0) {
		if (errno == C_EEXIST) {
			if ((c_sys_stat(s, &st) < 0) || !C_ISDIR(st.mode)) {
				errno = C_ENOTDIR;
				return -1;
			}
		} else {
			return -1;
		}
	}
	return 0;
}

ctype_status
mkpath(char *dir, uint mode, uint dmode)
{
	char *s;

	s = dir;
	if (*s == '/')
		++s;

	for (;;) {
		if (!(s = c_str_chr(s, C_USIZEMAX, '/')))
			break;
		*s = 0;
		if (makedir(dir, dmode) < 0)
			return c_err_warn("makedir %s", dir);
		*s++ = '/';
	}
	if (makedir(dir, mode) < 0)
		return c_err_warn("makedir %s", dir);

	return 0;
}
