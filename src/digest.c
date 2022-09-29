#include <tertium/cpu.h>
#include <tertium/std.h>

#include "common.h"

#define HDEC(x) ((x <= '9') ? x - '0' : (((uchar)x | 32) - 'a') + 10)

struct hash {
	ctype_hmd *md;
	int siz;
	char *name;
};

static ctype_arr arr;

static void
sethash(struct hash *p, char *s)
{
	if (!c_str_casecmp(s, -1, "MD5")) {
		p->name = "MD5";
		p->md = c_hsh_md5;
		p->siz = C_HSH_MD5DIG;
	} else if (!c_str_casecmp(s, -1, "SHA1")) {
		p->name = "SHA1";
		p->md = c_hsh_sha1;
		p->siz = C_HSH_SHA1DIG;
	} else if (!c_str_casecmp(s, -1, "SHA256")) {
		p->name = "SHA256";
		p->md = c_hsh_sha256;
		p->siz = C_HSH_SHA256DIG;
	} else if (!c_str_casecmp(s, -1, "SHA512")) {
		p->name = "SHA512";
		p->md = c_hsh_sha512;
		p->siz = C_HSH_SHA512DIG;
	} else if (!c_str_casecmp(s, -1, "WHIRLPOOL")) {
		p->name = "WHIRLPOOL";
		p->md = c_hsh_whirlpool;
		p->siz = C_HSH_WHIRLPOOLDIG;
	} else {
		c_err_diex(1, "%s: unknown algorithm", s);
	}
}

static ctype_status
hexcmp(char *p, usize n, char *s)
{
	while (n--) {
		if (((HDEC(p[0]) << 4) | HDEC(p[1])) != (uchar)*s++) return 1;
		p += 2;
	}
	return 0;
}

static ctype_status
checkfile(struct hash *h, char *file)
{
	ctype_hst hs;
	ctype_ioq ioq;
	usize n;
	ctype_fd fd;
	ctype_status r;
	char *p, *s;
	char buf[C_IOQ_SMALLBSIZ];
	char out[64];

	if ((fd = c_nix_fdopen2(file, C_NIX_OREAD)) < 0)
		c_err_die(1, "failed to open \"%s\"", file);

	r = 0;
	c_ioq_init(&ioq, fd, buf, sizeof(buf), c_nix_fdread);
	c_arr_trunc(&arr, 0, sizeof(uchar));
	while (c_ioq_getln(&ioq, &arr) > 0) {
		s = c_arr_data(&arr);
		n = c_arr_bytes(&arr);
		s[n - 1] = 0;

		if (!(p = c_mem_chr(s, n, ' ')))
			c_err_diex(1, "%s: file in wrong format", file);
		*p++ = 0;

		h->md->init(&hs);
		if (c_hsh_putfile(&hs, h->md, p) < 0) {
			r = c_err_warn("failed to read \"%s\"", s);
			c_arr_trunc(&arr, 0, sizeof(uchar));
			continue;
		}
		h->md->end(&hs, out);

		if (hexcmp(s, h->siz, out))
			r = c_err_warnx("%s %s: checksum mismatch", h->name, s);
		c_arr_trunc(&arr, 0, sizeof(uchar));
	}
	c_nix_fdclose(fd);
	return r;
}

static ctype_status
digest(struct hash *h, char *file)
{
	ctype_hst hs;
	int i;
	char buf[64];

	h->md->init(&hs);
	if (c_hsh_putfile(&hs, h->md, file) < 0)
		return c_err_warn("failed to read \"%s\"", file);
	h->md->end(&hs, buf);

	for (i = 0; i < h->siz; ++i) c_ioq_fmt(ioq1, "%02x", (uchar)buf[i]);
	c_ioq_fmt(ioq1, " %s\n", file);
	return 0;
}

static void
usage(void)
{
	c_ioq_fmt(ioq2, "usage: %s [-c] [-a algorithm] [file ...]\n",
	    c_std_getprogname());
	c_std_exit(1);
}

ctype_status
main(int argc, char **argv)
{
	struct hash h;
	ctype_status (*func)(struct hash *, char *);
	ctype_status r;


	if (!CSTRCMP("md5sum", argv[0])) {
		sethash(&h, "md5sum");
	} else if (!CSTRCMP("sha1sum", argv[0])) {
		sethash(&h, "SHA1SUM");
	} else if (!CSTRCMP("sha256sum", argv[0])) {
		sethash(&h, "SHA256SUM");
	} else if (!CSTRCMP("sha512sum", argv[0])) {
		sethash(&h, "SHA512SUM");
	} else {
		sethash(&h, "WHIRLPOOL");
	}

	c_std_setprogname(argv[0]);
	--argc, ++argv;

	func = digest;
	while (c_std_getopt(argmain, argc, argv, "a:c")) {
		switch (argmain->opt) {
		case 'a':
			sethash(&h, argmain->arg);
			break;
		case 'c':
			func = checkfile;
			break;
		default:
			usage();
		}
	}
	argc -= argmain->idx;
	argv += argmain->idx;
	if (!argc) argv = tmpargv("-");

	r = 0;
	for (; *argv; ++argv) {
		if (C_STD_ISDASH(*argv)) *argv = "<stdin>";
		r |= func(&h, *argv);
	}
	c_dyn_free(&arr);
	c_ioq_flush(ioq1);
	return r;
}
