#include <tertium/cpu.h>
#include <tertium/std.h>

#include "common.h"

static int
regcopy(char *src, ctype_stat *stp, char *dest)
{
	ctype_stat st;
	ctype_ioq ioq;
	ctype_fd ifd, ofd;
	int r;

	ifd = -1;
	r = 0;

	if ((ofd = c_sys_open(dest, C_OCREATE | C_OWRITE | C_OTRUNC,
	    stp->mode)) < 0) {
		r = c_err_warn("c_sys_open %s", dest);
		goto done;
	}

	if ((ifd = c_sys_open(src, C_OREAD, 0)) < 0) {
		r = c_err_warn("c_sys_open %s", src);
		goto done;
	}

	if (c_sys_fstat(&st, ofd) < 0) {
		r = c_err_warn("c_sys_fstat %s", src);
		goto done;
	}

	if (stp->dev == st.dev && stp->ino == st.ino) {
		r = c_err_warnx("%s %s: same file", src, dest);
		goto done;
	}

	c_ioq_init(&ioq, ofd, nil, 0, c_sys_write);

	if (c_ioq_putfd(&ioq, ifd, stp->size) < 0) {
		r = c_err_warn("c_ioq_putfd %s %s", src, dest);
		goto done;
	}

done:
	if (ifd != -1)
		c_sys_close(ifd);
	if (ofd != -1)
		c_sys_close(ofd);
	return r;
}

static int
lncopy(char *src, ctype_stat *stp, char *dest)
{
	size r;
	char buf[C_PATHMAX];

	if ((r = c_sys_readlink(buf, sizeof(buf) - 1, src)) < 0)
		return c_err_warn("readlink %s", src);

	buf[r] = 0;

	if ((size)stp->size < r)
		return c_err_warnx("%s: not same file\n", src);

	if (c_sys_symlink(buf, dest) < 0)
		return c_err_warn("c_sys_symlink %s %s", src, dest);

	return 0;
}

static int
ndcopy(char *s, ctype_stat *stp)
{
	if (c_sys_mknod(s, stp->mode, stp->dev) < 0)
		return c_err_warn("c_sys_mknod %s", s);

	return 0;
}

ctype_status
copy(char **argv, char *dest, uint ropts, uint opts)
{
	ctype_arr d;
	ctype_dir dir;
	ctype_dent *p;
	usize n;
	int rv;
	char buf[C_PATHMAX];

	if (c_dir_open(&dir, argv, ropts, nil) < 0)
		c_err_die(1, "c_dir_open");

	c_arr_init(&d, buf, sizeof(buf));
	c_arr_fmt(&d, "%s", dest);

	rv = 0;

	while ((p = c_dir_read(&dir))) {
		switch(p->info) {
		case C_FSD:
			if (!(opts & CP_RFLAG)) {
				c_dir_set(&dir, p, C_FSSKP);
				rv = c_err_warnx("%s: %s",
				    p->path, serr(C_EISDIR));
				continue;
			}
			if (p->depth || (opts & CP_TDIR))
				c_arr_fmt(&d, "/%s", p->name);
			if (c_sys_mkdir(c_arr_data(&d), p->stp->mode) < 0 &&
			    errno != C_EEXIST)
				rv = c_err_warn("c_sys_mkdir %s",
				    c_arr_data(&d));
			continue;
		case C_FSDP:
			c_arr_trunc(&d, c_arr_bytes(&d) - (p->nlen + 1),
			    sizeof(uchar));
			continue;
		case C_FSDNR:
		case C_FSERR:
		case C_FSNS:
			rv = c_err_warnx("%s: %s", p->path, serr(p->err));
			continue;
		}

		n = 0;
		if ((opts & CP_TDIR)) {
			n = c_arr_fmt(&d, "/%s",
			    p->depth ? p->name : c_gen_basename(sdup(p->name)));
		} else if (p->depth) {
			n = c_arr_fmt(&d, "/%s", p->name);
		}

		if (opts & CP_FFLAG)
			(void)c_sys_unlink(c_arr_data(&d));

		switch (p->info) {
		case C_FSF:
			rv = regcopy(p->path, p->stp, c_arr_data(&d));
			break;
		case C_FSSL:
			rv = lncopy(p->path, p->stp, c_arr_data(&d));
			break;
		default:
			rv = ndcopy(c_arr_data(&d), p->stp);
		}

		c_arr_trunc(&d, c_arr_bytes(&d) - n, sizeof(uchar));
	}

	c_dir_close(&dir);

	return rv;
}
