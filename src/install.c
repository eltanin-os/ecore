#include <tertium/cpu.h>
#include <tertium/std.h>

#include "common.h"

enum {
	DFLAG = 1 << 0,
	DDFLAG = 1 << 1,
};

static void
usage(void)
{
	c_ioq_fmt(ioq2,
	    "usage: %s [-Dd] [-g group] [-m mode] [-o owner] "
	    "source ... target\n",
	    c_std_getprogname());
	c_std_exit(1);
}

ctype_status
main(int argc, char **argv)
{
	struct install in;
	ctype_stat st;
	ctype_error sverr;
	ctype_status r;
	uint dmode, mode;
	uint mask, opts;
	char *dest, *s;

	c_std_setprogname(argv[0]);
	--argc, ++argv;

	opts = 0;
	mask = c_nix_getumask();
	mode = C_ACCESSPERMS & ~mask;
	dmode = mode | C_IWUSR | C_IXUSR;
	in.gid = in.mode = in.uid = -1;
	in.opts = CP_ATOMIC | CP_PFLAG;
	in.ropts = 0;

	while (c_std_getopt(argmain, argc, argv, "Dcdg:m:o:s")) {
		switch (argmain->opt) {
		case 'D':
			opts |= DDFLAG;
			break;
		case 'c':
			/* ignore */
			break;
		case 'd':
			opts |= DFLAG;
			break;
		case 'g':
			s = argmain->arg;
			if ((in.gid = gidfromname(s)) < 0)
				in.gid = estrtovl(s, 0, 0, C_UINTMAX);
			break;
		case 'm':
			mode = c_nix_strtomode(argmain->arg,
			    C_ACCESSPERMS, mask);
			in.mode = mode;
			break;
		case 'o':
			s = argmain->arg;
			if ((in.uid = uidfromname(s)) < 0)
				in.uid = estrtovl(s, 0, 0, C_UINTMAX);
			break;
		case 's':
			/* ignore */
			break;
		default:
			usage();
		}
	}
	argc -= argmain->idx;
	argv += argmain->idx;

	if (opts & DFLAG) {
		r = 0;
		for (; *argv; ++argv)
			if (c_nix_mkpath(*argv, mode, dmode) < 0)
				r = c_err_warn("c_nix_mkpath %s", *argv);
		c_std_exit(r);
	}

	if (argc < 2)
		usage();

	--argc;
	dest = argv[argc];
	argv[argc] = nil;
	if (opts & DDFLAG) {
		if (!(s = c_str_dup(dest, C_USIZEMAX)))
			c_err_die(1, "c_str_dup");
		if (c_gen_dirname(s) == s) {
			if (c_nix_mkpath(s, mode, dmode) < 0)
				c_err_die(1, "c_nix_mkpath %s", s);
			st.mode = C_IFDIR;
		} else {
			st.mode = 0;
		}
		c_std_free(s);
	}
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
		in.opts |= CP_TDIR;
	else if (argc > 1)
		usage();

	return install(&in, argv, dest);
}
