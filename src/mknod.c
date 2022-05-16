#include <tertium/cpu.h>
#include <tertium/std.h>

#include "common.h"

#define STR(a) estrtovl((a), 0, 0, C_LIM_ULONGMAX);
#define MAKEDEV(a, b) C_NIX_MAKEDEV(STR((a)), STR((b)))

static void
usage(void)
{
	c_ioq_fmt(ioq2,
	    "usage: %s [-m mode] file b|c major minor\n"
	    "       %s [-m mode] file p\n",
	    c_std_getprogname(), c_std_getprogname());
	c_std_exit(1);
}

ctype_status
main(int argc, char **argv)
{
	ulong dev;
	uint mask, mode, type;

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
	if (argc < 2 || argv[1][1]) usage();

	type = C_NIX_IFCHR;
	switch (argv[1][0]) {
	case 'b':
		type = C_NIX_IFBLK;
		/* FALLTHROUGH */
	case 'c':
		if (argc - 4) usage();
		dev = C_NIX_MAKEDEV(argv[2], argv[3]);
		if (c_sys_mknod(argv[0], type | mode, dev) < 0)
			c_err_die(1, "c_sys_mknod %s", argv[0]);
		break;
	case 'p':
		if (argc - 2) usage();
		if (c_sys_mknod(argv[0], C_NIX_IFIFO | mode, 0) < 0)
			c_err_die(1, "c_sys_mknod %s", argv[0]);
		break;
	default:
		usage();
	}
	return 0;
}
