#include <tertium/cpu.h>
#include <tertium/std.h>

#include "common.h"

#define CWTMODE (C_OCREATE | C_OWRITE | C_OTRUNC)

static ctype_status
regcopy(char *src, ctype_stat *stp, char *dest)
{
	ctype_stat st;
	ctype_ioq ioq;
	ctype_fd ifd, ofd;
	ctype_status r;

	ifd = -1;
	r = 0;
	if ((ofd = c_sys_open(dest, CWTMODE, stp->mode)) < 0) {
		r = c_err_warn("c_sys_open %s", dest);
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

	if ((ifd = c_sys_open(src, C_OREAD, 0)) < 0) {
		r = c_err_warn("c_sys_open %s", src);
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

static ctype_status
lncopy(char *src, ctype_stat *stp, char *dest)
{
	size r;
	char buf[C_PATHMAX];

	if ((r = c_sys_readlink(src, buf, sizeof(buf))) < 0)
		return c_err_warn("readlink %s", src);

	if ((size)stp->size < r)
		return c_err_warnx("%s: not same file\n", src);

	if (c_sys_symlink(buf, dest) < 0)
		return c_err_warn("c_sys_symlink %s %s", src, dest);

	return 0;
}

static ctype_status
ndcopy(char *s, ctype_stat *stp)
{
	if (c_sys_mknod(s, stp->mode, stp->dev) < 0)
		return c_err_warn("c_sys_mknod %s", s);

	return 0;
}

ctype_status
install(struct install *p, char **argv, char *dest)
{
	ctype_arr d;
	ctype_dir dir;
	ctype_dent *ep;
	ctype_stat st;
	ctype_status r;
	usize len, n;

	if (c_dir_open(&dir, argv, p->ropts, nil) < 0)
		c_err_die(1, "c_dir_open");

	c_mem_set(&d, sizeof(d), 0);
	edynfmt(&d, "%s", dest);
	n = r = 0;
	while ((ep = c_dir_read(&dir))) {
		c_arr_trunc(&d, c_arr_bytes(&d) - n, sizeof(uchar));
		n = 0;
		switch(ep->info) {
		case C_FSD:
			if (!(p->opts & CP_RFLAG)) {
				c_dir_set(&dir, ep, C_FSSKP);
				r = c_err_warnx("%s: %r", ep->path, C_EISDIR);
				continue;
			}
			if (ep->depth || (p->opts & CP_TDIR))
				edynfmt(&d, "/%s", ep->name);
			if (c_sys_mkdir(c_arr_data(&d), ep->stp->mode) < 0 &&
			    errno != C_EEXIST)
				r = c_err_warn("c_sys_mkdir %s",
				    c_arr_data(&d));
			continue;
		case C_FSDP:
			c_arr_trunc(&d, c_arr_bytes(&d) - (ep->nlen + 1),
			    sizeof(uchar));
			continue;
		case C_FSDNR:
		case C_FSERR:
		case C_FSNS:
			r = c_err_warnx("%s: %r", ep->path, ep->err);
			continue;
		}

		if (p->opts & CP_TDIR) {
			n = edynfmt(&d, "/%s",
			    ep->depth ? ep->name : c_gen_basename(ep->name));
		} else if (ep->depth) {
			n = edynfmt(&d, "/%s", ep->name);
		}

		if ((p->opts & CP_IFLAG) && prompt(c_arr_data(&d)))
			continue;

		if (p->opts & CP_FFLAG)
			c_sys_unlink(c_arr_data(&d));

		dest = c_arr_data(&d);
		if (p->opts & CP_ATOMIC) {
			len = c_arr_bytes(&d);
			if (c_dyn_ready(&d, len + 2, sizeof(uchar)) < 0)
				c_err_die(1, "c_dyn_ready");
			dest = (char *)c_arr_data(&d) + len + 1;
			if (c_mem_chr(c_arr_data(&d), len, '/')) {
				c_mem_cpy(dest, len, c_arr_data(&d));
				dest[len] = 0;
				c_gen_dirname(dest);
				len = c_str_len(dest, len);
				dest[len++] = '/';
			} else {
				len = 0;
			}
			c_mem_cpy(dest + len, 15, "INST@XXXXXXXXX");
			for (;;) {
				c_rand_name(dest + len + 5, 9);
				if (c_sys_stat(&st, dest) && errno == C_ENOENT)
					break;
			}
		}

		switch (ep->info) {
		case C_FSF:
			r = regcopy(ep->path, ep->stp, dest);
			break;
		case C_FSSL:
		case C_FSSLN:
			r = lncopy(ep->path, ep->stp, dest);
			break;
		default:
			r = ndcopy(dest, ep->stp);
		}

		if ((p->gid != -1 || p->uid != -1) &&
		    c_sys_chown(dest,
		    ID(p->uid, ep->stp->uid),
		    ID(p->gid, ep->stp->gid)) < 0) {
			r = c_err_warn("c_sys_chown %s", dest);
			continue;
		}
		if ((ep->info != C_FSSL) && (p->mode != (uint)-1) &&
		    c_sys_chmod(dest, p->mode) < 0) {
			r = c_err_warn("c_sys_chmod %s", dest);
			continue;
		}
		if ((p->opts & CP_ATOMIC) &&
		    c_sys_rename(dest, c_arr_data(&d)) < 0)
			r = c_err_warn("c_sys_rename %s %s",
			    dest, c_arr_data(&d));
	}
	c_dyn_free(&d);
	c_dir_close(&dir);
	return r;
}

ctype_status
copy(char **argv, char *dest, uint ropts, uint opts)
{
	struct install in;

	in.gid = in.mode = in.uid = -1;
	in.opts = opts;
	in.ropts = ropts;
	return install(&in, argv, dest);
}
