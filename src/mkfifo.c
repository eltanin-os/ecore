#include <tertium/cpu.h>
#include <tertium/std.h>

static void
usage(void)
{
	c_ioq_fmt(ioq2, "usage: %s [-m mode] file ...\n", c_std_getprogname());
	c_std_exit(1);
}

ctype_status
main(int argc, char **argv)
{
	ctype_status r;
	uint mask, mode;

	c_std_setprogname(argv[0]);
	--argc, ++argv;

	mask = c_nix_getumask();
	mode = C_NIX_DEFFILEMODE;

	while (c_std_getopt(argmain, argc, argv, "m:")) {
		switch (argmain->opt) {
		case 'm':
			mode = c_nix_strtomode(argmain->arg,
			    C_NIX_ACCESSPERMS, mask);
			break;
		default:
			usage();
		}
	}
	argc -= argmain->idx;
	argv += argmain->idx;
	if (!argc) usage();

	r = 0;
	for (; *argv; ++argv) {
		if (c_sys_mknod(*argv, C_NIX_IFIFO | mode, 0) < 0) {
			r = c_err_warn("failed to create FIFO \"%s\"", *argv);
		}
	}
	return r;
}
