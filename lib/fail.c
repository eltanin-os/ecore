#include <tertium/cpu.h>
#include <tertium/std.h>

#include "common.h"

size
edyncat(ctype_arr *p, void *v, usize m, usize n)
{
	size r;
	if ((r = c_dyn_cat(p, v, m, n)) < 0) c_err_die(1, nil);
	return r;
}

size
edynfmt(ctype_arr *p, char *fmt, ...)
{
	size r;
	va_list ap;
	va_start(ap, fmt);
	if ((r = c_dyn_vfmt(p, fmt, ap)) < 0) c_err_die(1, nil);
	va_end(ap);
	return r;
}

vlong
estrtovl(char *p, int b, vlong l, vlong h)
{
	ctype_status e;
	vlong r;
	r = c_std_strtovl(p, b, l, h, nil, &e);
	if (e < 0) c_err_die(1, "failed to transform string \"%s\"", p);
	return r;
}

uvlong
estrtouvl(char *p, int b, uvlong l, uvlong h)
{
	ctype_status e;
	vlong r;
	r = c_std_strtouvl(p, b, l, h, nil, &e);
	if (e < 0) c_err_die(1, "failed to transform string \"%s\"", p);
	return r;
}
