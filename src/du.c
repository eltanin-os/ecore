#include <tertium/cpu.h>
#include <tertium/std.h>

#include "common.h"

enum {
	AFLAG = 1 << 0,
	SFLAG = 1 << 1,
};

static uint ropts;
static uint opts;
static int  blksize = 512;

static int
du(Dir *p, char *path, uintptr *total)
{
	CDir dir;
	uintptr sbt;
	int r, rv;

	switch (dir_open(p, &dir, path, ropts)) {
	case -1:
		return c_err_warn("dir_open %s", path);
	case  1:
		sbt = C_HOWMANY(p->dp->info.st_blocks, blksize);
		c_ioq_fmt(ioq1, "%lld\t%s\n", sbt, path);
		/* fallthrough */
	case  2:
		return 0;
	}

	rv = 0;
	p->depth++;
	while ((r = c_dir_read(p->dp, &dir)) > 0) {
		sbt = C_HOWMANY(p->dp->info.st_blocks, blksize);
		if (C_ISDIR(p->dp->info.st_mode)) {
			if (du(p, p->dp->path, &sbt)) {
				rv = 1;
				continue;
			}
		} else if (opts & AFLAG) {
			switch (c_dir_hist(&p->hp, &p->dp->info)) {
			case -1:
				c_err_die(1, "c_dir_hist");
			case  1:
				continue;
			}
			c_ioq_fmt(ioq1, "%lld\t%s\n", sbt, p->dp->path);
		}
		*total += sbt;
	}
	p->depth--;

	if (r < 0)
		r = c_err_warn("c_dir_read %s", dir.path);

	if (!p->depth || !(opts & SFLAG))
		c_ioq_fmt(ioq1, "%lld\t%s\n", *total, dir.path);

	/* call c_dir_close directly to keep history intact */
	c_dir_close(&dir);

	return rv;
}

static void
usage(void)
{
	c_ioq_fmt(ioq2, "usage: %s [-kx] [-a|-s] [-H|-L] [file ...]\n",
	    c_std_getprogname());
	c_std_exit(1);
}

int
main(int argc, char **argv)
{
	Dir dir;
	uintptr n;
	int r;

	c_std_setprogname(argv[0]);

	C_ARGBEGIN {
	case 'H':
		ropts |= C_FSCOM;
		break;
	case 'L':
		ropts &= ~C_FSPHY;
		ropts |= C_FSLOG;
		break;
	case 'a':
		opts &= ~SFLAG;
		opts |= AFLAG;
		break;
	case 'k':
		blksize = 1024;
		break;
	case 's':
		opts &= ~AFLAG;
		opts |= SFLAG;
		break;
	case 'x':
		ropts |= C_FSXDV;
		break;
	default:
		usage();
	} C_ARGEND

	if (!argc) {
		argv[0] = ".";
		argv[1] = nil;
	}

	r = 0;
	blksize /= 512;
	c_mem_set(&dir, sizeof(dir), 0);

	for (; *argv; argc--, argv++) {
		n  = 0;
		r |= du(&dir, *argv, &n);
	}

	while (dir.hp)
		c_dst_lfree(c_dst_lpop(&dir.hp));

	c_ioq_flush(ioq1);

	return r;
}
