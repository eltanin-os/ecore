#include <tertium/cpu.h>
#include <tertium/std.h>

#include "common.h"

static void
getline(void)
{
	char buf;
	buf = 0;
	while (buf != '\n') c_ioq_get(&buf, sizeof(buf), ioq0);
}

int
yesno(char *msg, char *file)
{
	char buf;

	c_ioq_fmt(ioq2, "%s: %s '%s'? ", c_std_getprogname(), msg, file);
	c_ioq_flush(ioq2);

	c_ioq_get(&buf, sizeof(buf), ioq0);
	getline();
	return (buf | 32) != 'y';
}

int
prompt(char *s)
{
	ctype_stat st;
	if (c_nix_stat(&st, s) < 0) {
		if (errno == C_ERR_ENOENT) return 0;
		return c_err_warn("failed to obtain file info \"%s\"", s);
	}
	return yesno("overwrite", s);
}
