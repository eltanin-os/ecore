#include <tertium/cpu.h>
#include <tertium/std.h>

#include "common.h"

#define CWTMODE (C_NIX_OCREATE | C_NIX_OWRITE | C_NIX_OTRUNC)

/* copy routines */
static ctype_status
regcopy(char *src, ctype_stat *stp, char *dest)
{
	ctype_stat st;
	ctype_ioq ioq;
	ctype_fd ifd, ofd;
	ctype_status r;

	ifd = -1;
	r = 0;
	if ((ofd = c_nix_fdopen3(dest, CWTMODE, stp->mode)) < 0) {
		r = c_err_warn("c_nix_fdopen3 %s", dest);
		goto done;
	}

	if (c_nix_fdstat(&st, ofd) < 0) {
		r = c_err_warn("c_nix_fdstat %s", src);
		goto done;
	}

	if (stp->dev == st.dev && stp->ino == st.ino) {
		r = c_err_warnx("%s %s: same file", src, dest);
		goto done;
	}

	if ((ifd = c_nix_fdopen2(src, C_NIX_OREAD)) < 0) {
		r = c_err_warn("c_nix_fdopen2 %s", src);
		goto done;
	}

	c_ioq_init(&ioq, ofd, nil, 0, c_nix_fdwrite);
	if (c_ioq_putfd(&ioq, ifd, stp->size) < 0) {
		r = c_err_warn("c_ioq_putfd %s %s", src, dest);
		goto done;
	}
done:
	if (ifd != -1)
		c_nix_fdclose(ifd);
	if (ofd != -1)
		c_nix_fdclose(ofd);
	return r;
}

static ctype_status
lncopy(char *src, ctype_stat *stp, char *dest)
{
	size r;
	char buf[C_LIM_PATHMAX];

	if ((r = c_nix_readlink(buf, sizeof(buf), src)) < 0)
		return c_err_warn("c_nix_readlink %s", src);

	if ((size)stp->size < r) return c_err_warnx("%s: not same file\n", src);

	if (c_nix_symlink(dest, buf) < 0)
		return c_err_warn("c_nix_symlink %s <- %s", dest, buf);
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
		case C_DIR_FSD:
			if (!(p->opts & CP_RFLAG)) {
				c_dir_set(&dir, ep, C_DIR_FSSKP);
				r = c_err_warnx("%s: %r",
				    ep->path, C_ERR_EISDIR);
				continue;
			}
			if (ep->depth || (p->opts & CP_TDIR))
				edynfmt(&d, "/%s", ep->name);
			if (c_nix_mkdir(c_arr_data(&d), ep->stp->mode) < 0 &&
			    errno != C_ERR_EEXIST)
				r = c_err_warn("c_nix_mkdir %s",
				    c_arr_data(&d));
			continue;
		case C_DIR_FSDP:
			c_arr_trunc(&d, c_arr_bytes(&d) - (ep->nlen + 1),
			    sizeof(uchar));
			continue;
		case C_DIR_FSDNR:
		case C_DIR_FSERR:
		case C_DIR_FSNS:
			r = c_err_warnx("%s: %r", ep->path, ep->err);
			continue;
		}

		if (p->opts & CP_TDIR) {
			n = edynfmt(&d, "/%s",
			    ep->depth ? ep->name : c_gen_basename(ep->name));
		} else if (ep->depth) {
			n = edynfmt(&d, "/%s", ep->name);
		}

		if ((p->opts & CP_IFLAG) && prompt(c_arr_data(&d))) continue;

		if (p->opts & CP_FFLAG) c_nix_unlink(c_arr_data(&d));

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
				if (c_nix_stat(&st, dest) &&
				    errno == C_ERR_ENOENT)
					break;
			}
		}

		switch (ep->info) {
		case C_DIR_FSF:
			r = regcopy(ep->path, ep->stp, dest);
			break;
		case C_DIR_FSSL:
		case C_DIR_FSSLN:
			r = lncopy(ep->path, ep->stp, dest);
			break;
		default:
			r = ndcopy(dest, ep->stp);
		}

		if ((p->gid != -1 || p->uid != -1) &&
		    c_nix_chown(dest,
		    ID(p->uid, ep->stp->uid),
		    ID(p->gid, ep->stp->gid)) < 0) {
			r = c_err_warn("c_nix_chown %s", dest);
			continue;
		}
		if ((ep->info != C_DIR_FSSL) && (p->mode != (uint)-1) &&
		    c_nix_chmod(dest, p->mode) < 0) {
			r = c_err_warn("c_nix_chmod %s", dest);
			continue;
		}
		if ((p->opts & CP_ATOMIC) &&
		    c_nix_rename(c_arr_data(&d), dest) < 0)
			r = c_err_warn("c_nix_rename %s <- %s",
			    c_arr_data(&d), dest);
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

/* remove routines */
ctype_status
remove(char **argv, uint opts)
{
	ctype_dir dir;
	ctype_dent *p;
	ctype_status r;

	if (c_dir_open(&dir, argv, 0, nil) < 0) c_err_die(1, "c_dir_open");
	r = 0;
	while ((p = c_dir_read(&dir))) {
		if (p->info == C_DIR_FSNS) {
			if (!(opts & RM_FFLAG)) {
				errno = C_ERR_ENOENT;
				r = c_err_warn("remove %s", p->path);
			}
			continue;
		}
		if ((opts & RM_IFLAG) && yesno("remove", p->path)) continue;
		switch (p->info) {
		case C_DIR_FSD:
			if (!(opts & RM_RFLAG)) {
				c_dir_set(&dir, p, C_DIR_FSSKP);
				r = c_err_warnx("%s: %r",
				    p->path, C_ERR_EISDIR);
				continue;
			}
			break;
		case C_DIR_FSDP:
			if (!(opts & RM_RFLAG)) continue;
			if (c_nix_rmdir(p->path) < 0)
				r = c_err_warn("c_nix_rmdir %s", p->path);
			break;
		case C_DIR_FSERR:
			r = c_err_warnx("%s: %r", p->path, p->err);
			break;
		default:
			if (c_nix_unlink(p->path) < 0)
				r = c_err_warn("c_nix_unlink %s", p->path);
		}
	}
	c_dir_close(&dir);
	return r;
}
