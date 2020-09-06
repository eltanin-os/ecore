#include <tertium/cpu.h>
#include <tertium/std.h>

#define DISPLAY(a) c_ioq_fmt(ioq1, "%s%s", first++ ? " " : "", (a));

enum {
	AFLAG = 0x1F,
	MFLAG = 1 << 0,
	NFLAG = 1 << 1,
	RFLAG = 1 << 2,
	SFLAG = 1 << 3,
	VFLAG = 1 << 4,
};

static int first;

static void
usage(void)
{
	c_ioq_fmt(ioq2, "usage: %s [-amnrsv]\n", c_std_getprogname());
	c_std_exit(1);
}

ctype_status
main(int argc, char **argv)
{
	ctype_utsname uts;
	uint opts;

	c_std_setprogname(argv[0]);
	--argc, ++argv;

	opts = 0;

	while (c_std_getopt(argmain, argc, argv, "amnrsv")) {
		switch (argmain->opt) {
		case 'a':
			opts |= AFLAG;
			break;
		case 'm':
			opts |= MFLAG;
			break;
		case 'n':
			opts |= NFLAG;
			break;
		case 'r':
			opts |= RFLAG;
			break;
		case 's':
			opts |= SFLAG;
			break;
		case 'v':
			opts |= VFLAG;
			break;
		default:
			usage();
		}
	}
	argc -= argmain->idx;
	argv += argmain->idx;

	if (argc)
		usage();

	if (c_sys_uname(&uts) < 0)
		c_err_die(1, "c_sys_uname");

	if (!opts || (opts & SFLAG))
		DISPLAY(uts.sysname);
	if (opts & NFLAG)
		DISPLAY(uts.nodename);
	if (opts & RFLAG)
		DISPLAY(uts.release);
	if (opts & VFLAG)
		DISPLAY(uts.version);
	if (opts & MFLAG)
		DISPLAY(uts.machine);

	c_ioq_put(ioq1, "\n");
	c_ioq_flush(ioq1);
	return 0;
}
