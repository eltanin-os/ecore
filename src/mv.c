#include <tertium/cpu.h>
#include <tertium/std.h>

#include "common.h"

#define CPOPTS (CP_FFLAG | CP_PFLAG | CP_RFLAG)
#define RMOPTS (RM_FFLAG | RM_RFLAG)

static int tdir;

static int
move(char *src, char *dest)
{
	char *argv[2];

	if (!c_sys_rename(src, pathcat(src, dest, tdir)))
		return 0;

	if (errno != C_EXDEV)
		return c_err_warn("c_sys_rename %s %s", src, dest);

	argv[0] = src;
	argv[1] = nil;

	return (copy(argv, dest, 0, CPOPTS | tdir) || remove (argv, RMOPTS));
}

static void
usage(void)
{
	c_ioq_fmt(ioq2, "usage: %s [-if] source ... target\n",
	    c_std_getprogname());
	c_std_exit(1);
}

int
main(int argc, char **argv)
{
	ctype_stat st;
	int rv;
	char *dest;

	c_std_setprogname(argv[0]);

	C_ARGBEGIN {
	case 'i':
		/* ignore */
		break;
	case 'f':
		/* ignore */
		break;
	default:
		usage();
	} C_ARGEND

	if (argc < 2)
		usage();

	--argc;
	dest = argv[argc];
	argv[argc] = nil;

	if (c_sys_stat(&st, dest) < 0 && errno != C_ENOENT)
		c_err_die(1, "c_sys_stat %s", dest);
	else if (C_ISDIR(st.mode))
		tdir = CP_TDIR;
	else if (argc > 1)
		usage();

	rv = 0;

	for (; *argv; --argc, ++argv)
		rv |= move(*argv, dest);

	return 0;
}
