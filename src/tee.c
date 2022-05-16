#include <tertium/cpu.h>
#include <tertium/std.h>

static void
usage(void)
{
	c_ioq_fmt(ioq2, "usage: %s [-ai] [file ...]\n", c_std_getprogname());
	c_std_exit(1);
}

int
main(int argc, char **argv)
{
	ctype_fd *fds;
	size r;
	uint opts;
	int i;
	char buf[C_IOQ_BSIZ];

	c_std_setprogname(argv[0]);
	--argc, ++argv;

	opts = C_NIX_OWRITE | C_NIX_OTRUNC | C_NIX_OCREATE;

	while (c_std_getopt(argmain, argc, argv, "ai")) {
		switch (argmain->opt) {
		case 'a':
			opts = (opts & ~C_NIX_OTRUNC) | C_NIX_OAPPEND;
			break;
		case 'i':
			break;
		default:
			usage();
		}
	}
	argc -= argmain->idx;
	argv += argmain->idx;

	if (!(fds = c_std_alloc(argc + 1, sizeof(*fds))))
		c_err_die(1, "c_std_alloc");

	for (i = 0; i < argc; ++i)
		if ((fds[i] = c_nix_fdopen3(argv[i],
		    opts, C_NIX_DEFFILEMODE)) < 0)
			c_err_die(1, "c_nix_fdopen3 %s", argv[i]);

	fds[i] = C_IOQ_FD1;
	++argc;
	while ((r = c_nix_fdread(C_IOQ_FD0, buf, sizeof(buf))) > 0)
		for (i = 0; i < argc; ++i)
			if (c_nix_allrw(&c_nix_fdwrite, fds[i], buf, r) < 0)
				c_err_die(1, "c_nix_fdwrite %s",
				    argv[i] ? argv[i] : "<stdout>");
	if (r < 0) c_err_die(1, "c_nix_fdread");
	return 0;
}
