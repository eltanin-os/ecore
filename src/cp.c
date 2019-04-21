#include <tertium/cpu.h>
#include <tertium/std.h>

#include "common.h"

enum {
	FFLAG = 1 << 0,
	IFLAG = 1 << 1,
	PFLAG = 1 << 2,
};

static uint opts;

static int
copy(char *src, char *dest, ushort mode)
{
	CArr arr;
	CIoq ioq;
	int  fd;

	if (opts & FFLAG)
		c_sys_unlink(dest);

	if ((fd = c_sys_open(dest, O_CREAT|C_OWRITE|C_OEXCL, mode)) < 0)
		return c_err_warn("c_sys_open %s", dest);

	c_arr_init(&arr, nil, 0);
	c_ioq_init(&ioq, fd, &arr, c_sys_write);

	if (c_ioq_putfile(&ioq, src) < 0) {
		c_sys_close(fd);
		return c_err_warn("c_ioq_putfile %s %s", src, dest);
	}

	c_sys_close(fd);

	return 0;
}

static int
cp(Dir *p, char *src, char *dest)
{
	CDir dir;
	int r, rv;
	char buf[C_PATHMAX];

	switch (dir_open(p, &dir, src, 0)) {
	case -1:
		return c_err_warn("dir_open %s", src);
	case  1:
		return copy(src, dest, p->dp->info.st_mode);
	}

	c_sys_mkdir(dest, p->dp->info.st_mode);

	rv = 0;
	p->depth++;
	while ((r = c_dir_read(p->dp, &dir)) > 0) {
		pathcatf(buf, sizeof(buf), p->dp->name, dest, 1);
		if (C_ISDIR(p->dp->info.st_mode)) {
			if (cp(p, p->dp->path, buf)) {
				rv = 1;
				continue;
			}
		} else {
			rv |= copy(p->dp->path, buf, p->dp->info.st_mode);
		}
	}
	p->depth--;

	if (r < 0)
		rv = c_err_warn("c_dir_read %s", dir.path);

	dir_close(p, &dir);

	return rv;
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
	CStat st;
	Dir   dir;
	int   rv;
	char *dest;

	c_std_setprogname(argv[0]);

	c_mem_set(&dir, sizeof(dir), 0);
	dir.hp = (void *)-1;
	dir.maxdepth = 1;

	C_ARGBEGIN {
	case 'R':
		dir.maxdepth = 0;
		break;
	case 'f':
		opts |= FFLAG;
		break;
	case 'i':
		opts |= IFLAG;
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
		c_std_exit(cp(&dir, argv[0], pathcat(argv[0], argv[1])));
	}

	dest = argv[argc - 1];
	argv[argc - 1] = nil;
	if (c_sys_stat(&st, dest) < 0)
		c_err_die(1, "c_sys_stat %s", dest);

	if (!C_ISDIR(st.st_mode))
		usage();

	rv = 0;

	for (; *argv; argc--, argv++)
		rv |= cp(&dir, *argv, pathcatx(*argv, dest));

	return rv;
}
