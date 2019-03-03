#include <tertium/cpu.h>
#include <tertium/std.h>

#include "ec.h"

int
ec_err_warn(char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	ec_err_vwarn(fmt, ap);
	va_end(ap);
	return 1;
}

int
ec_err_vwarn(char *fmt, va_list ap)
{
	c_ioq_fmt(ioq2, "%s: ", c_std_getprogname());
	c_ioq_vfmt(ioq2, fmt, ap);
	c_ioq_fmt(ioq2, ": %r\n");
	c_ioq_flush(ioq2);
	return 1;
}
