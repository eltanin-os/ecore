#include <tertium/cpu.h>
#include <tertium/std.h>

#include "common.h"

enum {
	FFLAG = 1 << 0,
	SFLAG = 1 << 1,
	LFLAG = 1 << 2,
};

static ctype_status
link(char *dest, char *src, int opts)
{
	int flags = (opts & LFLAG) ? C_NIX_SLNOFLW : 0;
	if (opts & FFLAG) c_nix_unlink(dest);
	return c_sys_linkat(C_NIX_FDCWD, src, C_NIX_FDCWD, dest, flags);
}

static ctype_status
linkit(char *src, char *dest, uint opts)
{
	ctype_stat sta, stb;
	ctype_status (*statf)(ctype_stat *, char *);
	ctype_status error;

	if (opts & SFLAG) {
		if (opts & FFLAG) c_nix_unlink(dest);
		if (c_nix_symlink(dest, src) < 0) {
			return c_err_warn(
			    "failed to create symlink \"%s\" to \"%s\"",
			    src, dest);
		}
		return 0;
	}

	error = 0;
	if (!c_nix_lstat(&sta, dest)) {
		statf = (opts & LFLAG) ? c_nix_stat : c_nix_lstat;
		if (statf(&stb, src) < 0) {
			error = 1;
		} else if (sta.dev == stb.dev && sta.ino == stb.ino) {
			if (opts & FFLAG) return 0;
			c_std_werrstr("same file");
			error = 1;
		}
	}
	if (error || link(dest, src, opts) < 0) {
		return c_err_warn(
		    "failed to link \"%s\" to \"%s\"",
		    src, dest);
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
		c_std_exit(linkit(argv[0], pathcat(argv[0], ".", 1), opts));
	case 2:
		c_std_exit(linkit(argv[0], pathcat(argv[0], argv[1], 0), opts));
	}

	--argc;
	dest = argv[argc];
	argv[argc] = nil;
	if (c_nix_stat(&st, dest) < 0) {
		c_err_die(1, "failed to obtain file info \"%s\"", dest);
	}
	if (!C_NIX_ISDIR(st.mode)) usage();

	r = 0;
	for (; *argv; ++argv) r |= linkit(*argv, pathcat(*argv, dest, 1), opts);
	return r;
}
