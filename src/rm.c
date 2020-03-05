#include <tertium/cpu.h>
#include <tertium/std.h>

#include "common.h"

static void
usage(void)
{
	c_ioq_fmt(ioq2, "usage: %s [-ifRr] file ...\n", c_std_getprogname());
	c_std_exit(1);
}

ctype_status
main(int argc, char **argv)
{
	uint opts;

	c_std_setprogname(argv[0]);

	opts = 0;

	C_ARGBEGIN {
	case 'R':
	case 'r':
		opts |= RM_RFLAG;
		break;
	case 'f':
		opts |= RM_FFLAG;
		break;
	case 'i':
		/* ignore */
		break;
	default:
		usage();
	} C_ARGEND

	if (!argc) {
		if (!(opts & RM_FFLAG))
			usage();
		c_std_exit(0);
	}
	return remove(argv, opts);
}
