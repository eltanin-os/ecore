#include <tertium/cpu.h>
#include <tertium/std.h>

enum {
	AFLAG = 0x1F,
	MFLAG = 1 << 0,
	NFLAG = 1 << 1,
	RFLAG = 1 << 2,
	SFLAG = 1 << 3,
	VFLAG = 1 << 4,
};

static void
display(char *s)
{
	static int first;
	c_ioq_fmt(ioq1, "%s%s", first++ ? " " : "", s);
}

static void
usage(void)
{
	c_ioq_fmt(ioq2, "usage: %s [-amnrsv]\n", c_std_getprogname());
	c_std_exit(1);
}

int
main(int argc, char **argv)
{
	ctype_utsname uts;
	uint opts;

	c_std_setprogname(argv[0]);
	opts = 0;

	C_ARGBEGIN {
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
	} C_ARGEND

	if (argc)
		usage();

	if (c_sys_uname(&uts) < 0)
		c_err_die(1, "c_sys_uname");

	if (!opts || (opts & SFLAG))
		display(uts.sysname);
	if (opts & NFLAG)
		display(uts.nodename);
	if (opts & RFLAG)
		display(uts.release);
	if (opts & VFLAG)
		display(uts.version);
	if (opts & MFLAG)
		display(uts.machine);

	c_ioq_put(ioq1, "\n");
	c_ioq_flush(ioq1);

	return 0;
}
