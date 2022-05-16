#include <tertium/cpu.h>
#include <tertium/std.h>

#include "common.h"

enum {
	FFLAG = 1 << 0,
	SFLAG = 1 << 1,
	LFLAG = 1 << 2,
};

static ctype_status
linkit(char *src, char *dest, uint opts)
{
	if (opts & FFLAG) c_nix_unlink(dest);
	if (opts & SFLAG) {
		if (c_nix_symlink(dest, src) < 0)
			return c_err_warn("c_nix_symlink %s <- %s", dest, src);
	} else if (opts & LFLAG) {
		if (c_sys_linkat(C_NIX_FDCWD, src,
		    C_NIX_FDCWD, dest, C_NIX_SLNOFLW) < 0)
			return c_err_warn("c_sys_linkat %s %s", src, dest);
	} else {
		if (c_nix_link(dest, src) < 0)
			return c_err_warn("c_nix_link %s <- %s", dest, src);
	}
	return 0;
}

static void
usage(void)
{
	c_ioq_fmt(ioq2,
	    "usage: %s [-f] [-L|-P|-s] source [target]\n"
	    "       %s [-f] [-L|-P|-s] source ... directory\n",
	    c_std_getprogname(), c_std_getprogname());
	c_std_exit(1);
}

ctype_status
main(int argc, char **argv)
{
	ctype_stat st;
	ctype_status r;
	uint opts;
	char *dest;

	c_std_setprogname(argv[0]);
	--argc, ++argv;

	opts = 0;

	while (c_std_getopt(argmain, argc, argv, "LPfs")) {
		switch (argmain->opt){
		case 'L':
			opts = (opts & ~SFLAG) | LFLAG;
			break;
		case 'P':
			opts &= ~(LFLAG | SFLAG);
			break;
		case 'f':
			opts |= FFLAG;
			break;
		case 's':
			opts = (opts & ~LFLAG) | SFLAG;
			break;
		default:
			usage();
		}
	}
	argc -= argmain->idx;
	argv += argmain->idx;
	switch (argc) {
	case 0:
		usage();
	case 1:
		c_std_exit(linkit(argv[0], pathcat(argv[0], ".", 0), opts));
	case 2:
		c_std_exit(linkit(argv[0], pathcat(argv[0], argv[1], 0), opts));
	}

	--argc;
	dest = argv[argc];
	argv[argc] = nil;
	if (c_nix_stat(&st, dest) < 0) c_err_die(1, "c_nix_stat %s", dest);
	if (!C_NIX_ISDIR(st.mode)) usage();

	r = 0;
	for (; *argv; ++argv)
		r |= linkit(*argv, pathcat(*argv, dest, 1), opts);
	return r;
}
