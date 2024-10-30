#include <tertium/cpu.h>
#include <tertium/std.h>

#include "common.h"

/* create mode */
static void
cmode(char *path)
{
	ctype_cdbmk cdbmk;
	ctype_arr arr;
	usize klen, vlen;
	ctype_status r;
	ctype_fd tmpfd;
	char *s, *p, *tmp;

	c_mem_set(&arr, sizeof(arr), 0);
	if (c_dyn_fmt(&arr, "%s.tmp.XXXXXXXXX", path) < 0) {
		c_err_diex(1, "no memory");
	}
	c_dyn_shrink(&arr, sizeof(uchar));
	tmp = c_arr_data(&arr);

	if ((tmpfd = c_nix_mktemp(tmp, c_arr_bytes(&arr))) < 0) {
		c_err_die(1, "failed to obtain temporary file");
	}
	if (c_cdb_mkstart(&cdbmk, tmpfd) < 0) {
		c_err_warn("failed to initialize the database");
		goto error;
	}

	c_mem_set(&arr, sizeof(arr), 0);
	while ((r = c_ioq_getln(&arr, ioq0)) > 0) {
		if (*(s = c_arr_data(&arr)) == '\n') break;
		if (*s != '+') goto invalid;
		++s;
		if (!(p = c_str_chr(s, c_arr_bytes(&arr), ','))) goto invalid;
		*p++ = '\0';
		klen = estrtouvl(s, 10, 0, -1);
		s = p;
		if (!(p = c_str_chr(s, c_arr_bytes(&arr), ':'))) goto invalid;
		*p++ = '\0';
		vlen = estrtouvl(s, 10, 0, -1);

		s = p;
		p = s + klen;
		if (p[0] != '-' || p[1] != '>') goto invalid;
		p += 2;
		c_cdb_mkadd(&cdbmk, s, klen, p, vlen);
		c_arr_trunc(&arr, 0, sizeof(uchar));
		continue;
invalid:
		c_err_warnx("input in wrong format");
		goto error;
	}
	c_dyn_free(&arr);

	if (c_cdb_mkfinish(&cdbmk) < 0) {
		c_err_warn("failed to write the database");
		goto error;
	}
	c_nix_fdclose(tmpfd);

	if (c_nix_rename(path, tmp) < 0) {
		c_err_warn("failed to move \"%s\" to \"%s\"", tmp, path);
		goto error;
	}
	return;
error:
	c_nix_unlink(tmp);
	c_std_exit(1);
}

/* query mode */
static ctype_status
qmode(ctype_fd fd, char *key, usize n)
{
	ctype_arr arr;
	ctype_cdb cdb;
	usize len;
	ctype_status r;
	int found;

	if (c_cdb_init(&cdb, fd) < 0) {
		c_err_die(1, "failed to initialize the database");
	}
	c_mem_set(&arr, sizeof(arr), 0);
	found  = 0;
	len = c_str_len(key, -1);
	while ((r = c_cdb_findnext(&cdb, key, len)) > 0) {
		if (!n--) break;
		++found;
		if (c_dyn_ready(&arr, c_cdb_datalen(&cdb), sizeof(uchar)) < 0) {
			c_err_diex(1, "no memory");
		}
		if (c_cdb_read(&cdb, c_arr_data(&arr),
		    c_cdb_datalen(&cdb), c_cdb_datapos(&cdb)) < 0) {
			c_err_die(1, "failed to read the database");
		}
		c_ioq_nput(ioq1, c_arr_data(&arr), c_cdb_datalen(&cdb));
	}
	c_dyn_free(&arr);
	if (r < 0) c_err_die(1, "failed to search the next key");
	return found ? 0 : 100;
}

/* d/s functions */
static void
getall(ctype_ioq *p, char *s, usize n)
{
	if (c_ioq_get(s, n, p) < 0) c_err_die(1, "truncated file");
}

static void
getseek(ctype_ioq *p, usize n)
{
	char buf;
	for (; n; --n) getall(p, &buf, 1);
}

static void
get32num(ctype_ioq *p, u32 *x)
{
	char buf[4];
	getall(p, buf, sizeof(buf));
	*x = c_uint_32unpack(buf);
}

/* dump mode */
static void
dmode(ctype_fd fd)
{
	ctype_ioq ioq;
	u32 eod, pos;
	u32 dlen, klen;
	char buf[C_IOQ_SMALLBSIZ];

	c_ioq_init(&ioq, fd, buf, sizeof(buf), c_nix_fdread);
	get32num(&ioq, &eod);
	getseek(&ioq, 2044);
	pos = 2048;
	while (pos < eod) {
		get32num(&ioq, &klen);
		get32num(&ioq, &dlen);
		c_ioq_fmt(ioq1, "+%d,%d:", klen, dlen);
		c_ioq_tofrom(ioq1, &ioq, klen);
		c_ioq_put(ioq1, "->");
		c_ioq_tofrom(ioq1, &ioq, dlen);
		c_ioq_put(ioq1, "\n");
		pos += dlen + klen + 8;
	}
	c_ioq_put(ioq1, "\n");
}

/* stats mode */
static void
smode(ctype_fd fd)
{
	ctype_cdb c;
	ctype_ioq ioq;
	ctype_fssize rest;
	u32 numrec, numd[11];
	u32 eod, pos;
	u32 dlen, klen;
	char key[1024];
	char buf[C_IOQ_SMALLBSIZ];

	c_cdb_init(&c, fd);
	c_mem_set(&numd, sizeof(numd), 0);
	numrec = 0;
	c_ioq_init(&ioq, fd, buf, sizeof(buf), c_nix_fdread);
	get32num(&ioq, &eod);
	getseek(&ioq, 2044);
	pos = 2048;
	while (pos < eod) {
		get32num(&ioq, &klen);
		pos += 4;
		get32num(&ioq, &dlen);
		pos += 4;
		if (klen > sizeof(key)) {
			getseek(&ioq, klen);
			pos += klen;
		} else {
			getall(&ioq, key, klen);
			pos += klen;
			rest = c_nix_seek(fd, 0, C_NIX_SEEKCUR);
			c_cdb_findstart(&c);
			do {
				switch (c_cdb_findnext(&c, key, klen)) {
				case -1:
					c_err_die(1, nil);
				case 0:
					c_err_diex(1, "truncated file");
				}
			} while (c_cdb_datapos(&c) != pos);
			if (!c.loop) c_err_diex(1, "truncated file");
			++numrec;
			++numd[c.loop > 10 ? 10 : c.loop - 1];
			c_nix_seek(fd, rest, C_NIX_SEEKSET);
		}
		getseek(&ioq, dlen);
		pos += dlen;
	}
	c_ioq_fmt(ioq1,
	    "records %d\n"
	    "d0      %d\n"
	    "d1      %d\n"
	    "d2      %d\n"
	    "d3      %d\n"
	    "d4      %d\n"
	    "d5      %d\n"
	    "d6      %d\n"
	    "d7      %d\n"
	    "d8      %d\n"
	    "d9      %d\n"
	    ">9      %d\n",
	    numrec, numd[0], numd[1], numd[2], numd[3], numd[4], numd[5],
	    numd[6], numd[7], numd[8], numd[9], numd[10]);
}

static void
usage(void)
{
	c_ioq_fmt(ioq2,
	    "usage: %s -cds file\n"
	    "       %s [-n num] -q key file\n",
	    c_std_getprogname(), c_std_getprogname());
	c_std_exit(1);
}

ctype_status
main(int argc, char **argv)
{
	usize n;
	ctype_fd fd;
	ctype_status r;
	int mode;
	char *key;

	c_std_setprogname(argv[0]);
	--argc, ++argv;

	key = nil;
	mode = 0;
	n = -1;

	while (c_std_getopt(argmain, argc, argv, "cdn:q:s")) {
		switch (argmain->opt) {
		case 'q':
			key = argmain->arg;
			/* FALLTHROUGH */
		case 'c':
		case 'd':
		case 's':
			mode = argmain->opt;
			break;
		case 'n':
			n = estrtouvl(argmain->arg, 0, 0, -1);
			break;
		default:
			usage();
		}
	}
	argc -= argmain->idx;
	argv += argmain->idx;

	r = 0;
	if (mode == 'c') {
		if (argc - 1) usage();
		cmode(*argv);
	} else {
		if (argc - 1) usage();
		fd = c_nix_fdopen2(*argv, C_NIX_OREAD);
		if (fd < 0) c_err_die(1, "failed to open \"%s\"", *argv);
		switch (mode) {
		case 'd':
			dmode(fd);
			break;
		case 'q':
			r = qmode(fd, key, n);
			break;
		case 's':
			smode(fd);
		}
		c_nix_fdclose(fd);
	}
	c_ioq_flush(ioq1);
	return r;
}
