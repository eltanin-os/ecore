#include <tertium/cpu.h>
#include <tertium/std.h>

#define display(a, b) \
c_ioq_fmt(ioq1, "%lld\t%s\n", C_HOWMANY((b), blksize), (a));

enum {
	AFLAG = 1 << 0,
	SFLAG = 1 << 1,
	XFLAG = 1 << 2,
};

static uint ropts;
static uint opts;
static int  blksize = 512;

static size
du(char *path, int depth)
{
	CDir  dir;
	CStat st;
	size  r;
	usize sz;
	int   rv;

	if (c_dir_open(&dir, path, ropts) < 0) {
		if (errno != ENOTDIR)
			return -c_err_warn("c_dir_open %s", path);
		if (c_sys_stat(&st, path))
			return -c_err_warn("c_sys_stat %s", path);
		if (!depth || (opts & AFLAG))
			display(path, st.st_blocks);
		return st.st_size;
	}

	dir.depth = depth;
	rv = 0;
	sz = 0;

	while ((r = c_dir_read(&dir))) {
		if (r < 0) {
			rv = -c_err_warn("c_dir_read %s", dir.path);
			continue;
		}
		sz += dir.info.st_blocks;
		if (!(depth+1) || (opts & AFLAG))
			display(dir.path, dir.info.st_blocks);
		if (C_ISDIR(dir.info.st_mode)) {
			if ((r = du(dir.path, depth+1)) < 0) {
				rv = r;
				continue;
			}
			sz += r;
		}
	}

	if (!depth || (~opts & SFLAG))
		display(path, sz);

	c_dir_close(&dir);

	return rv ? rv : sz;
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

	r = 0;
	blksize /= 512;

	if (!argc) {
		argv[0] = ".";
		argv[1] = nil;
	}

	for (; *argv; argc--, argv++) {
		if (du(*argv, 0) < 0)
			r = 1;
	}

	c_ioq_flush(ioq1);

	return r;
}
