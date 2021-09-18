#include <tertium/cpu.h>
#include <tertium/std.h>

#include "common.h"

#define CPOPTS (CP_ATOMIC | CP_PFLAG | CP_RFLAG)
#define RMOPTS (RM_FFLAG | RM_RFLAG)

static ctype_status
move(char *src, char *dest, uint opts)
{
	char *argv[2];
	char *s;

	s = pathcat(src, dest, opts & CP_TDIR);
	if ((opts & CP_IFLAG) && prompt(s))
		return 0;

	if (!c_nix_rename(s, src))
		return 0;
	if (errno != C_EXDEV)
		return c_err_warn("c_nix_rename %s <- %s", s, src);

	argv[0] = src;
	argv[1] = nil;
	return (copy(argv, dest, 0, CPOPTS | opts) || remove(argv, RMOPTS));
}

static void
usage(void)
{
	c_ioq_fmt(ioq2, "usage: %s [-if] source ... target\n",
	    c_std_getprogname());
	c_std_exit(1);
}

ctype_status
main(int argc, char **argv)
{
	ctype_stat st;
	ctype_error sverr;
	ctype_status r;
	uint opts;
	char *dest;

	c_std_setprogname(argv[0]);
	--argc, ++argv;

	opts = 0;

	while (c_std_getopt(argmain, argc, argv, "if")) {
		switch (argmain->opt) {
		case 'i':
			opts |= (opts & ~CP_FFLAG) | CP_IFLAG;
			break;
		case 'f':
			opts |= (opts & ~CP_IFLAG) | CP_FFLAG;
			break;
		default:
			usage();
		}
	}
	argc -= argmain->idx;
	argv += argmain->idx;

	if (argc < 2)
		usage();

	--argc;
	dest = argv[argc];
	argv[argc] = nil;
	if (c_nix_stat(&st, dest) < 0) {
		sverr = errno;
		if (c_nix_lstat(&st, dest) < 0) {
			errno = sverr;
			if (errno != C_ENOENT)
				c_err_die(1, "c_nix_stat %s", dest);
			st.mode = 0;
		}
	}
	if (C_ISDIR(st.mode))
		opts |= CP_TDIR;
	else if (argc > 1)
		usage();

	r = 0;
	for (; *argv; --argc, ++argv)
		r |= move(*argv, dest, opts);

	return 0;
}
