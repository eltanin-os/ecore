#include <tertium/cpu.h>
#include <tertium/std.h>

enum {
	AFLAG = 1 << 0,
	SFLAG = 1 << 1,
};

static CNode *hp;
static uint   ropts;
static uint   opts;
static int    blksize = 512;

static int
du(char *path, int depth, usize *n)
{
	CDir  dir;
	usize sbt;
	int   r, rv;

	if (c_dir_open(&dir, path, ropts) < 0) {
		if (errno != C_ENOTDIR)
			return c_err_warn("c_dir_open %s", path);
		sbt = C_HOWMANY(dir.info.st_blocks, blksize);
		c_ioq_fmt(ioq1, "%lld\t%s\n", sbt, path);
		return 0;
	}

	if ((r = c_dir_hist(&hp, &dir.info))) {
		c_dir_close(&dir);
		return (r < 0);
	}

	rv = 0;
	depth++;
	while ((r = c_dir_read(&dir)) > 0) {
		sbt = C_HOWMANY(dir.info.st_blocks, blksize);
		if (C_ISDIR(dir.info.st_mode)) {
			if (du(dir.path, depth, &sbt) < 0) {
				rv = 1;
				continue;
			}
		} else if (opts & AFLAG) {
			switch (c_dir_hist(&hp, &dir.info)) {
			case -1:
				c_err_die(1, "c_dir_hist");
			case  1:
				continue;
			}
			c_ioq_fmt(ioq1, "%lld\t%s\n", sbt, dir.path);
		}
		*n += sbt;
	}
	depth--;

	if (r < 0)
		rv = c_err_warn("c_dir_read %s", dir.path);

	if (!depth || !(opts & SFLAG))
		c_ioq_fmt(ioq1, "%lld\t%s\n", *n, path);

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
	usize n;
	int   r;

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

	for (; *argv; argc--, argv++) {
		n  = 0;
		r |= du(*argv, 0, &n);
	}

	while (hp)
		c_dst_lfree(c_dst_lpop(&hp), nil);

	c_ioq_flush(ioq1);

	return r;
}
