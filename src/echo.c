#include <tertium/cpu.h>
#include <tertium/std.h>

int
main(int argc, char **argv)
{
	int nflag;

	c_std_setprogname(argv[0]);
	argc--, argv++;

	nflag = 0;

	if (!c_mem_cmp(*argv, sizeof("-n"), "-n")) {
		argc--, argv++;
		nflag++;
	}

	for (; *argv; argc--, argv++) {
		c_ioq_put(ioq1, *argv);
		if (argc-1)
			c_ioq_put(ioq1, " ");
	}

	if (!nflag)
		c_ioq_put(ioq1, "\n");

	c_ioq_flush(ioq1);

	return 0;
}
