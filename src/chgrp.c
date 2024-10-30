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
	c_ioq_fmt(ioq2, "usage: %s [-h] [-R [-H|-L|-P]] group file ...\n",
	    c_std_getprogname());
	c_std_exit(1);
}

ctype_status
main(int argc, char **argv)
{
	ctype_dir dir;
	ctype_dent *p;
	ctype_status r;
	ctype_id gid;
	uint opts, ropts;

	c_std_setprogname(argv[0]);
	--argc, ++argv;

	opts = 0;
	ropts = 0;

	while (c_std_getopt(argmain, argc, argv, "HLPRh")) {
		switch (argmain->opt) {
		case 'H':
			opts |= C_DIR_FSCOM;
			break;
		case 'L':
			opts = (opts & ~C_DIR_FSPHY) | C_DIR_FSLOG;
			break;
		case 'P':
			opts = (opts & ~C_DIR_FSLOG) | C_DIR_FSPHY;
			break;
		case 'R':
			ropts |= RFLAG;
			break;
		case 'h':
			ropts |= HFLAG;
			break;
		default:
			usage();
		}
	}
	argc -= argmain->idx;
	argv += argmain->idx;
	if (argc < 2) usage();

	if (!(ropts & RFLAG)) {
		opts = (ropts & HFLAG) ? C_DIR_FSCOM : 0;
	}

	if ((gid = gidfromname(argv[0])) < 0) {
		gid = estrtovl(argv[0], 0, 0, C_LIM_UINTMAX);
	}

	++argv;
	if (c_dir_open(&dir, argv, opts, nil) < 0) c_err_die(1, nil);
	r = 0;
	while ((p = c_dir_read(&dir))) {
		switch (p->info) {
		case C_DIR_FSD:
			if (!(ropts & RFLAG)) c_dir_set(&dir, p, C_DIR_FSSKP);
			break;
		case C_DIR_FSDNR:
			r = c_err_warnx("%s: %r", p->name, p->err);
			continue;
		case C_DIR_FSDP:
			continue;
		case C_DIR_FSERR:
		case C_DIR_FSNS:
			r = c_err_warnx("%s: %r", p->name, p->err);
			continue;
		case C_DIR_FSSL:
			if (c_nix_lchown(p->path, p->stp->uid, gid) < 0) {
				r = c_err_warn("%s", p->path);
			}
			continue;
		case C_DIR_FSSLN:
			continue;
		}
		if (c_nix_chown(p->path, p->stp->uid, gid) < 0) {
			r = c_err_warn("%s", p->path);
		}
	}
	c_dir_close(&dir);
	return r;
}
