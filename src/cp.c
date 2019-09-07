#include <tertium/cpu.h>
#include <tertium/std.h>

#include "common.h"

static void
usage(void)
{
	c_ioq_fmt(ioq2, "usage: %s [-fip] [-R [-H|-L|-P]] source ... target\n",
	    c_std_getprogname());
	c_std_exit(1);
}

int
main(int argc, char **argv)
{
	ctype_stat st;
	uint opts, ropts;
	char *dest;

	c_std_setprogname(argv[0]);

	opts = 0;
	ropts = 0;

	C_ARGBEGIN {
	case 'H':
		ropts |= C_FSCOM;
		break;
	case 'L':
		ropts &= ~C_FSPHY;
		ropts |= C_FSLOG;
	case 'P':
		ropts &= ~C_FSLOG;
		ropts |= C_FSPHY;
		break;
	case 'R':
		opts |= CP_RFLAG;
		break;
	case 'f':
		opts |= CP_FFLAG;
		break;
	case 'i':
		/* ignore */
		break;
	case 'p':
		opts |= CP_PFLAG;
		break;
	default:
		usage();
	} C_ARGEND

	if (argc < 2)
		usage();

	argc--;
	dest = argv[argc];
	argv[argc] = nil;

	if (c_sys_stat(&st, dest) < 0 && errno != C_ENOENT)
		c_err_die(1, "c_sys_stat %s", dest);
	else if (C_ISDIR(st.mode))
		opts |= CP_TDIR;
	else if (argc > 1)
		usage();

	return copy(argv, dest, ropts, opts);
}
