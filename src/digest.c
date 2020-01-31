#include <tertium/cpu.h>
#include <tertium/std.h>

#include "common.h"

#define HDEC(x) ((x <= '9') ? x - '0' : (((uchar)x | 32) - 'a') + 10)

struct hash {
	ctype_hmd *md;
	int siz;
	char *name;
};

static void
sethash(struct hash *p, char *s)
{
	if (!CSTRCMP("MD5", s)) {
		p->name = "MD5";
		p->md = c_hsh_md5;
		p->siz = C_HMD5_DIGEST;
	} else if (!CSTRCMP("SHA1", s)) {
		p->name = "SHA1";
		p->md = c_hsh_sha1;
		p->siz = C_HSHA1_DIGEST;
	} else if (!CSTRCMP("SHA256", s)) {
		p->name = "SHA256";
		p->md = c_hsh_sha256;
		p->siz = C_HSHA256_DIGEST;
	} else if (!CSTRCMP("SHA512", s)) {
		p->name = "SHA512";
		p->md = c_hsh_sha512;
		p->siz = C_HSHA512_DIGEST;
	} else if (!CSTRCMP("WHIRLPOOL", s)) {
		p->name = "WHIRLPOOL";
		p->md = c_hsh_whirlpool;
		p->siz = C_HWHIRLPOOL_DIGEST;
	} else {
		c_err_diex(1, "%s: unknown algorithm", s);
	}
}

static int
cmpsum(struct hash *hp, ctype_hst *p, char *s)
{
	char buf[64];
	int i;

	c_hsh_digest(p, hp->md, buf);
	for (i = 0; i < hp->siz; ++i) {
		if (((HDEC(s[0]) << 4) | HDEC(s[1])) != (uchar)buf[i])
			return -1;
		s += 2;
	}

	return 0;
}

static int
checkfile(struct hash *hp, char *file)
{
	ctype_arr arr;
	ctype_fd fd;
	ctype_hst hs;
	ctype_ioq ioq;
	usize n;
	int r;
	char *p, *s;
	char buf[C_BIOSIZ];

	if ((fd = c_sys_open(file, C_OREAD, 0)) < 0)
		c_err_die(1, "c_sys_open %s", file);

	c_ioq_init(&ioq, fd, buf, sizeof(buf), c_sys_read);
	c_mem_set(&arr, sizeof(arr), 0);

	r = 0;
	while (c_ioq_getln(&ioq, &arr) > 0 ) {
		s = c_arr_data(&arr);
		n = c_arr_bytes(&arr);
		s[n - 1] = 0;

		if (!(p = c_mem_chr(s, n, ' ')))
			c_err_diex(1, "%s: file in wrong format", file);

		*p++ = 0;
		sethash(hp, s);

		n -= p - s;
		s = p;
		if (!(p = c_mem_chr(s, n, ' ')))
			c_err_diex(1, "%s: file in wrong format", file);

		*p++ = 0;
		hp->md->init(&hs);
		if (c_hsh_putfile(&hs, hp->md, p) < 0)
			r = c_err_warn("c_hsh_putfile %s", s);
		hp->md->end(&hs);
		if (cmpsum(hp, &hs, s) < 0)
			r = c_err_warnx("%s: checksum mismatch", s);

		c_arr_trunc(&arr, 0, sizeof(uchar));
	}
	c_dyn_free(&arr);
	return r;
}

static void
usage(void)
{
	c_ioq_fmt(ioq2, "usage: %s [-c] [-a algorithm] [file ...]\n",
	    c_std_getprogname());
	c_std_exit(1);
}

int
main(int argc, char **argv)
{
	struct hash hst;
	ctype_hst hs;
	int cflag;
	int i, r;
	char buf[64];

	c_std_setprogname(argv[0]);

	cflag = 0;

	C_ARGBEGIN {
	case 'a':
		sethash(&hst, C_EARGF(usage()));
		break;
	case 'c':
		cflag = 1;
		break;
	default:
		usage();
	} C_ARGEND

	if (!hst.siz)
		sethash(&hst, "WHIRLPOOL");

	if (!argc)
		argv = tmpargv("-");

	r = 0;
	if (cflag) {
		for (; *argv; ++argv) {
			if (C_ISDASH(*argv))
				*argv = "<stdin>";
			r |= checkfile(&hst, *argv);
		}
	} else {
		for (; *argv; ++argv) {
			if (C_ISDASH(*argv))
				*argv = "<stdin>";
			if (c_hsh_putfile(&hs, hst.md, *argv) < 0) {
				r = c_err_warn("c_hsh_putfile %s", *argv);
				continue;
			}
			c_hsh_digest(&hs, hst.md, buf);
			c_ioq_fmt(ioq1, "%s ", hst.name);
			for (i = 0; i < hst.siz; ++i)
				c_ioq_fmt(ioq1, "%02x", (uchar)buf[i]);
			c_ioq_fmt(ioq1, " %s\n", *argv);
		}
	}
	c_ioq_flush(ioq1);
	return r;
}
