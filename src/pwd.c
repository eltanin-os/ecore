#include <tertium/cpu.h>
#include <tertium/std.h>

static char *
getpwd(void)
{
	CStat pwd, dot;
	usize n;
	char *s;

	if (!(s = c_sys_getenv("PWD")))
		return nil;

	if (*s != '/')
		return nil;

	if ((n = c_str_len(s)) >= C_PATHMAX)
		return nil;

	if (c_mem_mem(s, n, "/./",  sizeof("/./")-1) ||
	    c_mem_mem(s, n, "/../", sizeof("/../")-1))
		return nil;

	if (c_sys_stat(&pwd, s)   < 0 ||
	    c_sys_stat(&dot, ".") < 0)
		return nil;

	if (pwd.st_dev != dot.st_dev ||
	    pwd.st_ino != dot.st_ino)
		return nil;

	return s;
}

static void
usage(void)
{
	c_ioq_fmt(ioq2, "usage: %s [-L|-P]\n", c_std_getprogname());
	c_ioq_flush(ioq2);
	c_std_exit(1);
}

int
main(int argc, char **argv)
{
	int  lpflag;
	char buf[C_PATHMAX];
	char *s;

	c_std_setprogname(argv[0]);

	lpflag = 'L';

	C_ARGBEGIN {
	case 'L':
	case 'P':
		lpflag = C_ARGC();
		break;
	default:
		usage();
	} C_ARGEND

	if (argc)
		usage();

	s = lpflag == 'L' ? getpwd() : nil;
	if (!s && !(s = c_sys_getcwd(buf, sizeof(buf))))
		c_err_die(1, "c_sys_getcwd");

	c_ioq_fmt(ioq1, "%s\n", s);
	c_ioq_flush(ioq1);

	return 0;
}
