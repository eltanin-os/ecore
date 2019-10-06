#include <tertium/cpu.h>
#include <tertium/std.h>

#include "common.h"

static int
mkpath(char *dir, uint mode, uint dmode)
{
	char *s;

	s = dir;

	for (;;) {
		if (!(s = c_str_chr(s, C_USIZEMAX, '/')))
			break;
		*s = 0;
		if (c_sys_mkdir(dir, dmode) < 0 && errno != C_EEXIST)
			return c_err_warn("c_sys_mkdir %s", dir);
		*s++ = '/';
	}

	if (c_sys_mkdir(dir, mode) < 0 && errno != C_EEXIST)
		return c_err_warn("c_sys_mkdir %s", dir);

	return 0;
}

static void
usage(void)
{
	c_ioq_fmt(ioq2, "usage: %s [-p] [-m mode] dir ...\n",
	    c_std_getprogname());
	c_std_exit(1);
}

int
main(int argc, char **argv)
{
	int pflag;
	int rv;
	uint dmode, mode;
	uint mask;

	c_std_setprogname(argv[0]);

	mask = c_sys_umask(0);
	mode = C_ACCESSPERMS & ~mask;
	dmode = mode | C_IWUSR | C_IXUSR;
	pflag = 0;

	C_ARGBEGIN {
	case 'p':
		pflag = 1;
		break;
	case 'm':
		mode = strtomode(C_EARGF(usage()), C_ACCESSPERMS, mask);
		break;
	default:
		usage();
	} C_ARGEND

	if (!argc)
		usage();

	for (; *argv; --argc, ++argv) {
		trim_trailing_slash(*argv);
		if (pflag)
			rv |= mkpath(*argv, mode, dmode);
		else if (c_sys_mkdir(*argv, mode) < 0)
			rv = c_err_warn("c_sys_mkdir %s", *argv);
	}

	return rv;
}
