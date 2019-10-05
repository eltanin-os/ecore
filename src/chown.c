#include <tertium/cpu.h>
#include <tertium/std.h>

#include "common.h"

enum {
	HFLAG = 1 << 0,
	RFLAG = 1 << 1,
};

static void
usage(void)
{
	c_ioq_fmt(ioq2,
	    "usage: %s [-h] [-R [-H|-L|-P]] owner[:group] file ...\n",
	    c_std_getprogname());
	c_std_exit(1);
}

int
main(int argc, char **argv)
{
	ctype_dir dir;
	ctype_dent *p;
	int rv;
	uint uid;
	uint opts, ropts;

	c_std_setprogname(argv[0]);

	opts = 0;
	ropts = 0;

	C_ARGBEGIN {
	case 'H':
		opts |= C_FSCOM;
		break;
	case 'h':
		ropts |= HFLAG;
		break;
	case 'L':
		opts &= ~C_FSPHY;
		opts |= C_FSLOG;
		break;
	case 'P':
		opts &= ~C_FSLOG;
		opts |= C_FSPHY;
		break;
	case 'R':
		ropts |= RFLAG;
		break;
	default:
		usage();
	} C_ARGEND

	if (argc < 2)
		usage();

	if (!(ropts & RFLAG))
		opts = (ropts & HFLAG) ? C_FSCOM : 0;

	uid = estrtovl(argv[0], 0, 0, C_UINTMAX);

	++argv;
	if (c_dir_open(&dir, argv, opts, nil) < 0)
		c_err_die(1, "c_dir_open");

	rv = 0;

	while ((p = c_dir_read(&dir))) {
		switch (p->info) {
		case C_FSD:
			if (!(ropts & RFLAG))
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
			if (c_sys_lchown(p->path, uid, p->stp->gid) < 0)
				rv = c_err_warn("c_sys_chown %s", p->path);
			continue;
		case C_FSSLN:
			continue;
		}

		if (c_sys_chown(p->path, uid, p->stp->gid) < 0)
			rv = c_err_warn("c_sys_chown %s", p->path);
	}

	c_dir_close(&dir);

	return rv;
}
