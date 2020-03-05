#include <tertium/cpu.h>
#include <tertium/std.h>

ctype_status
main(int argc, char **argv)
{
	int nflag;

	c_std_setprogname(argv[0]);
	--argc, ++argv;

	nflag = 0;
	if (*argv && !c_str_cmp(*argv, C_USIZEMAX, "-n")) {
		nflag = 1;
		--argc, ++argv;
	}

	for (; *argv; --argc, ++argv) {
		c_ioq_put(ioq1, *argv);
		if (argc - 1)
			c_ioq_put(ioq1, " ");
	}

	if (!nflag)
		c_ioq_put(ioq1, "\n");

	c_ioq_flush(ioq1);
	return 0;
}
