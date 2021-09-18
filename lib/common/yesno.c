#include <tertium/cpu.h>
#include <tertium/std.h>

#include "common.h"

int
yesno(char *msg, char *file)
{
	int ans;
	char ch;

	c_ioq_fmt(ioq2, "%s: %s '%s'? ", c_std_getprogname(), msg, file);
	c_ioq_flush(ioq2);
	c_ioq_get(ioq0, &ch, 1);
	ans = (ch | 32) != 'y';
	while (ch != '\n') c_ioq_get(ioq0, &ch, 1);
	return ans;
}

int
prompt(char *s)
{
	ctype_stat st;

	if (c_nix_stat(&st, s) < 0) {
		if (errno == C_ENOENT)
			return 0;
		return c_err_warn("c_nix_stat %s", s);
	}

	return yesno("overwrite", s);
}
