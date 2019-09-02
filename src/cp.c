#include <tertium/cpu.h>
#include <tertium/std.h>

#include "common.h"

enum {
	FFLAG = 1 << 0,
	PFLAG = 1 << 1,
	RFLAG = 1 << 2,
};

static int
copy(char *src, char *dest, ushort mode)
{
	CIoq ioq;
	int  fd;

	if ((fd = c_sys_open(dest, C_OCREATE|C_OWRITE|C_OEXCL, mode)) < 0)
		return c_err_warn("c_sys_open %s", dest);

	c_ioq_init(&ioq, fd, arr_zero, c_sys_write);

	if (c_ioq_putfile(&ioq, src) < 0) {
		c_sys_close(fd);
		return c_err_warn("c_ioq_putfile %s %s", src, dest);
	}

	c_sys_close(fd);

	return 0;
}

static void
usage(void)
{
	c_ioq_fmt(ioq2, "usage: %s [-fip] [-R [-H|-L|-P]] source ... target\n",
	    c_std_getprogname());
	c_std_exit(1);
}

int
main(int argc, char **argv)
{
	CDir   dir;
	CDent *p;
	CStat  st;
	char  *dest, *ds;
	int    rv;
	uint   opts, ropts;

	c_std_setprogname(argv[0]);

	c_mem_set(&st, sizeof(st), 0);
	opts = 0;
	ropts = 0;

	C_ARGBEGIN {
	case 'H':
		ropts |= C_FSCOM;
		break;
	case 'L':
		ropts &= ~C_FSPHY;
		ropts |= C_FSLOG;
	case 'P':
		ropts &= ~C_FSLOG;
		ropts |= C_FSPHY;
		break;
	case 'R':
		opts |= RFLAG;
		break;
	case 'f':
		opts |= FFLAG;
		break;
	case 'i':
		/* ignore */
		break;
	case 'p':
		opts |= PFLAG;
		break;
	default:
		usage();
	} C_ARGEND

	switch (argc) {
	case 0:
	case 1:
		usage();
	case 2:
		argc--;
		dest = argv[argc];
		argv[argc] = nil;
		break;
	case 3:
		argc--;
		dest = argv[argc];
		argv[argc] = nil;

		if (c_sys_stat(&st, dest) < 0)
			c_err_die(1, "c_sys_stat %s", dest);

		if (!C_ISDIR(st.mode))
			usage();
	}

	if (c_dir_open(&dir, argv, ropts, nil) < 0)
		c_err_die(1, "c_dir_open");

	rv = 0;

	while ((p = c_dir_read(&dir))) {
		ds = pathcat(p->name, dest, st.mode);

		switch (p->info) {
		case C_FSD:
			if (!(opts & RFLAG)) {
				c_dir_set(&dir, p, C_FSSKP);
				rv = c_err_warnx("%s: %s",
				    p->path, serr(C_EISDIR));
				continue;
			}
			if (c_sys_mkdir(ds, p->stp->mode) < 0)
				rv = c_err_warn("c_sys_mkdir %s", ds);
			continue;
		case C_FSDP:
			continue;
		case C_FSERR:
		case C_FSNS:
			rv = c_err_warnx("%s: %s", p->path, serr(p->errno));
			continue;
		}

		if (opts & FFLAG)
			(void)c_sys_unlink(ds);

		rv = copy(p->path, ds, p->stp->mode);
	}

	c_dir_close(&dir);

	return rv;
}
