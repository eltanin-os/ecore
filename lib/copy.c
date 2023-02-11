#include <tertium/cpu.h>
#include <tertium/std.h>

#include "common.h"

#define FFLAG (p->opts & CP_FFLAG)
#define IFLAG (p->opts & CP_IFLAG)
#define PFLAG (p->opts & CP_PFLAG)
#define RFLAG (p->opts & CP_RFLAG)
#define TDIR  (p->opts & CP_TDIR)

ctype_status
install(struct install *p, char **argv, char *dest)
{
	ctype_arr d;
	ctype_dir dir;
	ctype_dent *ep;
	ctype_status r;
	usize n;

	if (c_dir_open(&dir, argv, p->ropts, nil) < 0) c_err_die(1, nil);
	c_mem_set(&d, sizeof(d), 0);
	edynfmt(&d, "%s", dest);
	n = r = 0;
	while ((ep = c_dir_read(&dir))) {
		c_arr_trunc(&d, c_arr_bytes(&d) - n, sizeof(uchar));
		n = 0;
		switch(ep->info) {
		case C_DIR_FSD:
			if (!RFLAG) {
				c_dir_set(&dir, ep, C_DIR_FSSKP);
				r = c_err_warnx("%s: %r",
				    ep->path, C_ERR_EISDIR);
				continue;
			}
			if (ep->depth || (p->opts & CP_TDIR))
				edynfmt(&d, "/%s", ep->name);
			if (c_nix_mkdir(c_arr_data(&d), ep->stp->mode) < 0 &&
			    errno != C_ERR_EEXIST)
				r = c_err_warn("failed to create dir \"%s\"",
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

		if (TDIR) {
			dest = ep->depth ? ep->name : c_gen_basename(ep->name);
			n = edynfmt(&d, "/%s", dest);
		} else if (ep->depth) {
			n = edynfmt(&d, "/%s", ep->name);
		}
		dest = c_arr_data(&d);

		if (IFLAG && prompt(dest)) continue;
		if (FFLAG) c_nix_unlink(dest);

		if (p->uid != -1) ep->stp->uid = p->uid;
		if (p->gid != -1) ep->stp->gid = p->gid;
		if (c_nix_fscopy(dest, PFLAG, ep->path, ep->stp) < 0) {
			r = c_err_warn("failed to copy \"%s\" to \"%s\"",
			    ep->path, dest);
			continue;
		}
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
