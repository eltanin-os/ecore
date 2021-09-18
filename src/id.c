#include <tertium/cpu.h>
#include <tertium/std.h>

#include "common.h"

enum {
	GGFLAG = 1 << 0,
	GFLAG = 1 << 1,
	UFLAG = 1 << 2,
	NFLAG = 1 << 3,
	RFLAG = 1 << 4,

	/* union flags */
	UF0 = GGFLAG | GFLAG | UFLAG,
	UF1 = NFLAG | RFLAG,
};

static int putch;

static void
pgrp(ctype_id id, uint opts)
{
	if (opts & (GGFLAG | GFLAG)) {
		if (putch)
			c_ioq_put(ioq1, " ");
		if (opts & NFLAG)
			c_ioq_fmt(ioq1, "%s", namefromgid(id));
		else
			c_ioq_fmt(ioq1, "%ud", id);
	} else {
		if (putch)
			c_ioq_put(ioq1, ",");
		c_ioq_fmt(ioq1, "%ud(%s)", id, namefromgid(id));
	}
	putch = 1;
}

static void
printgroups(char *name, uint opts)
{
	struct pdb db;
	ctype_arr arr;
	ctype_ioq ioq;
	ctype_fd fd;
	usize len, n;
	char buf[C_BIOSIZ];
	char *s;

	if ((fd = c_nix_fdopen2(GRPFILE, C_OREAD)) < 0)
		c_err_die(1, "c_nix_fdopen2 " GRPFILE);

	c_ioq_init(&ioq, fd, buf, sizeof(buf), &c_nix_fdread);
	c_mem_set(&arr, sizeof(arr), 0);
	n = c_str_len(name, C_USIZEMAX);
	while (dbgetln(&db, &ioq, &arr) > 0) {
		s = db.p[3];
		len = c_arr_bytes(&arr) - (db.p[3] - db.p[0]);
		while ((s = c_str_str(s, len, name))) {
			if ((s[n] == ',' || s[n] == '\n') &&
			    (s[-1] == ',' || s[-1] == '\0')) {
				pgrp(estrtovl(db.p[2], 0, 0, C_UINTMAX), opts);
				break;
			}
		}
	}
	c_dyn_free(&arr);
}

static void
usage(void)
{
	c_ioq_fmt(ioq2, "usage: %s [-G|-g|-u [-nr]] [user]\n",
	    c_std_getprogname());
	c_std_exit(1);
}

ctype_status
main(int argc, char **argv)
{
	ctype_id gid, id, uid;
	uint opts;
	char *p, *s, *t;

	c_std_setprogname(argv[0]);
	--argc, ++argv;

	opts = 0;

	while (c_std_getopt(argmain, argc, argv, "Ggnru")) {
		switch (argmain->opt) {
		case 'G':
			opts = (opts & ~(GFLAG | UFLAG)) | GGFLAG;
			break;
		case 'g':
			opts = (opts & ~(GGFLAG | UFLAG)) | GFLAG;
			break;
		case 'n':
			opts |= NFLAG;
			break;
		case 'r':
			opts |= RFLAG;
			break;
		case 'u':
			opts = (opts & ~(GGFLAG | GFLAG)) | UFLAG;
			break;
		default:
			usage();
		}
	}
	argc -= argmain->idx;
	argv += argmain->idx;

	if (argc > 1 || ((opts & UF1) && !(opts & UF0)))
		usage();

	if ((id = c_sys_geteuid()) < 0)
		c_err_die(1, "c_sys_geteuid");

	if (argc) {
		if (*argv[0] >= '0' && *argv[0] <= '9')
			uid = estrtovl(*argv, 0, 0, C_UINTMAX);
		else if ((uid = uidfromname(*argv)) < 0)
			c_err_diex(1, "%s: user not found", *argv);
	} else {
		if (opts & RFLAG) {
			if ((uid = c_sys_getuid()) < 0)
				c_err_die(1, "c_sys_getuid");
		} else {
			uid = id;
		}
	}

	if (opts & (GGFLAG | GFLAG)) {
		gid = gidfromuid(uid);
		if (opts & NFLAG)
			c_ioq_fmt(ioq1, "%s", namefromgid(gid));
		else
			c_ioq_fmt(ioq1, "%ud", gid);

		if (opts & GGFLAG) {
			c_ioq_put(ioq1, " ");
			printgroups(namefromuid(uid), opts);
		}
	} else if (opts & UFLAG) {
		if (opts & NFLAG)
			c_ioq_fmt(ioq1, "%s", namefromuid(uid));
		else
			c_ioq_fmt(ioq1, "%ud", uid);
	} else {
		gid = gidfromuid(uid);
		p = namefromgid(gid);
		s = namefromuid(uid);
		c_ioq_fmt(ioq1, "uid=%ud(%s) gid=%ud(%s) ", uid, s, gid, p);
		if (!argc && uid != id) {
			t = namefromuid(id);
			c_ioq_fmt(ioq1, "euid=%ud(%s) ", id, t);
			if ((id = gidfromuid(id)) != gid) {
				t = namefromgid(id);
				c_ioq_fmt(ioq1, "egid=%ud(%s) ", id, t);
			}
		}
		c_ioq_fmt(ioq1, "groups=%ud(%s)", gid, p);
		++putch;
		printgroups(s, opts);
	}
	c_ioq_put(ioq1, "\n");
	c_ioq_flush(ioq1);
	return 0;
}
