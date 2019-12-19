#include <tertium/cpu.h>
#include <tertium/std.h>

#include "common.h"

static int
makedir(char *s, uint mode)
{
	ctype_stat st;

	if (c_sys_mkdir(s, mode) < 0) {
		if (errno == C_EEXIST) {
			if ((c_sys_stat(&st, s) < 0) || C_ISDIR(st.mode)) {
				errno = C_ENOTDIR;
				return -1;
			}
		} else {
			return -1;
		}
	}

	return 0;
}

static int
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

	rv = 0;

	for (; *argv; --argc, ++argv) {
		trim_trailing_slash(*argv);
		if (pflag)
			rv |= mkpath(*argv, mode, dmode);
		else if (c_sys_mkdir(*argv, mode) < 0)
			rv = c_err_warn("c_sys_mkdir %s", *argv);
	}

	return rv;
}
