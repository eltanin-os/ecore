#include <tertium/cpu.h>
#include <tertium/std.h>

#include "common.h"

enum {
	AFLAG = 1 << 0,
	SFLAG = 1 << 1,
};

static void
usage(void)
{
	c_ioq_fmt(ioq2, "usage: %s [-kx] [-a|-s] [-H|-L] [file ...]\n",
	    c_std_getprogname());
	c_std_exit(1);
}

ctype_status
main(int argc, char **argv)
{
	ctype_dir dir;
	ctype_dent *p;
	ctype_status r;
	int blksiz;
	uint opts, ropts;

	c_std_setprogname(argv[0]);
	--argc, ++argv;

	blksiz = 512;
	opts = 0;
	ropts = 0;

	while (c_std_getopt(argmain, argc, argv, "HLaksx")) {
		switch (argmain->opt) {
		case 'H':
			ropts |= C_DIR_FSCOM;
			break;
		case 'L':
			ropts &= ~C_DIR_FSPHY;
			ropts |= C_DIR_FSLOG;
			break;
		case 'a':
			opts &= ~SFLAG;
			opts |= AFLAG;
			break;
		case 'k':
			blksiz = 1024;
			break;
		case 's':
			opts &= ~AFLAG;
			opts |= SFLAG;
			break;
		case 'x':
			ropts |= C_DIR_FSXDV;
			break;
		default:
			usage();
		}
	}
	argc -= argmain->idx;
	argv += argmain->idx;
	if (!argc) argv = tmpargv(".");

	if (c_dir_open(&dir, argv, ropts | C_DIR_FSFHT, nil) < 0)
		c_err_die(1, "c_dir_open");
	blksiz /= 512;
	r = 0;
	while ((p = c_dir_read(&dir))) {
		switch (p->info) {
		case C_DIR_FSD:
		case C_DIR_FSDC:
			break;
		case C_DIR_FSDP:
			if (!(opts & SFLAG) || !p->depth)
				c_ioq_fmt(ioq1, "%lld\t%s\n",
				    C_STD_HOWMANY(p->num, blksiz), p->path);
			p->parent->num += p->num;
			break;
		case C_DIR_FSFC:
			break;
		case C_DIR_FSDNR:
		case C_DIR_FSNS:
		case C_DIR_FSERR:
			r = c_err_warnx("%s: %r", p->path, p->err);
			break;
		default:
			if ((opts & AFLAG) || !p->depth)
				c_ioq_fmt(ioq1, "%lld\t%s\n",
				    C_STD_HOWMANY(p->stp->blocks, blksiz),
				    p->path);
			p->parent->num += p->stp->blocks;
		}
	}
	c_ioq_flush(ioq1);
	return r;
}
