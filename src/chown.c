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

ctype_status
main(int argc, char **argv)
{
	ctype_dir dir;
	ctype_dent *p;
	ctype_status r;
	ctype_id gid, uid;
	uint opts, ropts;
	char *grp;

	c_std_setprogname(argv[0]);
	--argc, ++argv;

	opts = 0;
	ropts = 0;

	while (c_std_getopt(argmain, argc, argv, "HLPRh")) {
		switch (argmain->opt) {
		case 'H':
			opts |= C_FSCOM;
			break;
		case 'L':
			opts = (opts & ~C_FSPHY) | C_FSLOG;
			break;
		case 'P':
			opts = (opts & ~C_FSLOG) | C_FSPHY;
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

	if (argc < 2)
		usage();

	if (!(ropts & RFLAG))
		opts = (ropts & HFLAG) ? C_FSCOM : 0;

	gid = -1;
	if ((grp = c_str_chr(argv[0], C_USIZEMAX, ':'))) {
		*grp++ = 0;
		if ((gid = gidfromname(grp)) < 0)
			gid = estrtovl(grp, 0, 0, C_UINTMAX);
	}

	if ((uid = uidfromname(argv[0])) < 0)
		uid = estrtovl(argv[0], 0, 0, C_UINTMAX);

	++argv;
	if (c_dir_open(&dir, argv, opts, nil) < 0)
		c_err_die(1, "c_dir_open");

	r = 0;
	while ((p = c_dir_read(&dir))) {
		switch (p->info) {
		case C_FSD:
			if (!(ropts & RFLAG))
				c_dir_set(&dir, p, C_FSSKP);
			break;
		case C_FSDNR:
			r = c_err_warnx("%s: %r", p->name, p->err);
			continue;
		case C_FSDP:
			continue;
		case C_FSERR:
		case C_FSNS:
			r = c_err_warnx("%s: %r", p->name, p->err);
			continue;
		case C_FSSL:
			if (c_sys_lchown(p->path,
			    uid, ID(gid, p->stp->gid)) < 0)
				r = c_err_warn("c_sys_chown %s", p->path);
			continue;
		case C_FSSLN:
			continue;
		}

		if (c_sys_chown(p->path, uid, ID(gid, p->stp->gid)) < 0)
			r = c_err_warn("c_sys_chown %s", p->path);
	}
	c_dir_close(&dir);
	return r;
}
