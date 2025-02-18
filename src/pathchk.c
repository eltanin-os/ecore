#include <tertium/cpu.h>
#include <tertium/std.h>

enum {
	PPFLAG = 1 << 0,
	PFLAG  = 1 << 1,
};

static usize pathmax = C_LIM_PATHMAX;
static usize namemax = C_LIM_NAMEMAX;

static int
check(uchar ch)
{
	return (ch - 'a' > 52) && (ch - '0' > 9) && !c_str_chr("._-", 3, ch);
}

static ctype_status
pathchk(char *path, uint opts)
{
	ctype_stat st;
	ctype_status r;
	usize len, n;
	char *s;

	r = 0;
	if ((opts & PFLAG) && !*path) r =  c_err_warnx("empty pathname");
	s = path;
	for (len = n = 0; *s; ++len, ++n, ++s) {
		if (*s == '/') {
			if ((opts & PFLAG) && (uchar)*(s + 1) == '-') {
				r = c_err_warnx("leading <hyphen-minus>"
				    " found in component of pathname", path);
			}
			if (n > namemax) {
				errno = C_ERR_ENAMETOOLONG;
				r = c_err_warn("%s", path);
			}
			n = 0;
			continue;
		}
		if ((opts & PPFLAG) && check(*s)) {
			r = c_err_warnx("non-portable character found in"
			    " component of pathname", path);
		}
	}
	if (n > namemax || len > pathmax) {
		errno = C_ERR_ENAMETOOLONG;
		r = c_err_warn("%s", path);
	}

	if ((c_nix_lstat(&st, s) < 0) && errno != C_ERR_ENOENT) {
		r = c_err_warn("failed to obtain file info \"%s\"", path);
	}
	return r;
}

static void
usage(void)
{
	c_ioq_fmt(ioq2, "usage: %s [-Pp] pathname ...\n", c_std_getprogname());
	c_std_exit(1);
}

ctype_status
main(int argc, char **argv)
{
	ctype_status r;
	uint opts;

	c_std_setprogname(argv[0]);
	--argc, ++argv;

	opts = 0;
	r = 0;

	while (c_std_getopt(argmain, argc, argv, "Pp")) {
		switch (argmain->opt) {
		case 'P':
			opts |= PFLAG;
			break;
		case 'p':
			opts |= PPFLAG;
			namemax = 14;
			pathmax = 256;
			break;
		default:
			usage();
		}
	}
	argc -= argmain->idx;
	argv += argmain->idx;
	if (!argc) usage();

	r = 0;
	for (; *argv; ++argv) r |= pathchk(*argv, opts);
	return r;
}
