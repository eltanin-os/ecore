#include <tertium/cpu.h>
#include <tertium/std.h>

#include "common.h"

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
	ulong maj, min;
	uint mode, type;

	c_std_setprogname(argv[0]);

	mode = C_DEFFILEMODE;

	C_ARGBEGIN {
	case 'm':
		mode = strtomode(C_EARGF(usage()), C_ACCESSPERMS, 0);
		break;
	default:
		usage();
	} C_ARGEND

	if (argc < 2 || argv[1][0])
		usage();

	type = C_IFCHR;
	switch (argv[1][0]) {
	case 'b':
		type = C_IFBLK;
		/* FALLTHROUGH */
	case 'c':
		if (argc - 4)
			usage();
		maj = estrtovl(argv[2], 0, 0, C_ULONGMAX);
		min = estrtovl(argv[3], 0, 0, C_ULONGMAX);
		if (c_sys_mknod(argv[0], type | mode, C_MAKEDEV(maj, min)) < 0)
			c_err_die(1, "c_sys_mknod %s", argv[0]);
		break;
	case 'p':
		if (argc - 2)
			usage();
		if (c_sys_mknod(argv[0], C_IFIFO | mode, 0) < 0)
			c_err_die(1, "c_sys_mknod %s", argv[0]);
		break;
	default:
		usage();
	}
	return 0;
}
