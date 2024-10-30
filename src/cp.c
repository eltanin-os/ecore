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

ctype_status
main(int argc, char **argv)
{
	ctype_stat st;
	ctype_error sverr;
	uint opts, ropts;
	char *dest;

	c_std_setprogname(argv[0]);
	--argc, ++argv;

	opts = 0;
	ropts = 0;

	while (c_std_getopt(argmain, argc, argv, "HLPRfipr")) {
		switch (argmain->opt) {
		case 'H':
			ropts |= C_DIR_FSCOM;
			break;
		case 'L':
			ropts = (ropts & ~C_DIR_FSPHY) | C_DIR_FSLOG;
			break;
		case 'P':
			ropts = (ropts & ~C_DIR_FSLOG) | C_DIR_FSPHY;
			break;
		case 'R':
			opts |= CP_RFLAG;
			break;
		case 'f':
			opts |= (opts & ~CP_IFLAG) | CP_FFLAG;
			break;
		case 'i':
			opts |= (opts & ~CP_FFLAG) | CP_IFLAG;
			break;
		case 'p':
			opts |= CP_PFLAG;
			break;
		case 'r':
			opts |= CP_RFLAG;
			break;
		default:
			usage();
		}
	}
	argc -= argmain->idx;
	argv += argmain->idx;
	if (argc < 2) usage();

	if (!ropts) ropts = (opts & CP_RFLAG) ? C_DIR_FSPHY : C_DIR_FSLOG;
	--argc;
	dest = argv[argc];
	argv[argc] = nil;
	if (c_nix_stat(&st, dest) < 0) {
		sverr = errno;
		if (c_nix_lstat(&st, dest) < 0) {
			errno = sverr;
			if (errno != C_ERR_ENOENT) {
				c_err_die(1,
				    "failed to obtain file info \"%s\"", dest);
			}
			st.mode = 0;
		}
	}
	if (C_NIX_ISDIR(st.mode)) {
		opts |= CP_TDIR;
	} else if (argc > 1) {
		usage();
	}
	return copy(argv, dest, ropts, opts);
}
