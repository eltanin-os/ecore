#include <tertium/cpu.h>
#include <tertium/std.h>

#include "common.h"

static void
usage(void)
{
	c_ioq_fmt(ioq2, "usage: %s [-R] mode file ...\n", c_std_getprogname());
	c_std_exit(1);
}

int
main(int argc, char **argv)
{
	ctype_dir dir;
	ctype_dent *p;
	int Rflag, rv;
	uint mask;
	char *m;

	c_std_setprogname(argv[0]);

	Rflag = 0;

	C_ARGBEGIN {
	case 'R':
		Rflag = 1;
		break;
	case 'X':
	case 'r':
	case 's':
	case 't':
	case 'x':
	case 'w':
		--argv[0];
		goto done;
	default:
		usage();
	} C_ARGEND
done:
	if (argc < 2)
		usage();

	c_sys_umask(mask = c_sys_umask(0));
	m = *argv;
	--argc, ++argv;

	if (c_dir_open(&dir, argv, 0, nil) < 0)
		c_err_die(1, "c_dir_open");

	rv = 0;

	while ((p = c_dir_read(&dir))) {
		switch (p->info) {
		case C_FSD:
			if (!Rflag)
				c_dir_set(&dir, p, C_FSSKP);
			break;
		case C_FSDNR:
			rv = c_err_warnx("%s: %s", p->name, serr(p->err));
			continue;
		case C_FSDP:
			continue;
		case C_FSERR:
		case C_FSNS:
			rv = c_err_warnx("%s: %s", p->name, serr(p->err));
			continue;
		case C_FSSL:
		case C_FSSLN:
			continue;
		}

		if (c_sys_chmod(p->path, strtomode(m, p->stp->mode, mask)) < 0)
			rv = c_err_warn("c_sys_chmod %s", p->path);
	}

	c_dir_close(&dir);

	return rv;
}
