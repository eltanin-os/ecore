#include <tertium/cpu.h>
#include <tertium/std.h>

#include "common.h"

#define FFLAG (opts & RM_FFLAG)
#define IFLAG (opts & RM_IFLAG)
#define RFLAG (opts & RM_RFLAG)

#define MSG "remove file"

ctype_status
remove(char **argv, uint opts)
{
	ctype_dir dir;
	ctype_dent *p;
	ctype_status r;

	if (c_dir_open(&dir, argv, 0, nil) < 0) c_err_die(1, nil);
	r = 0;
	while ((p = c_dir_read(&dir))) {
		if (p->info == C_DIR_FSNS) {
			if (!FFLAG) {
				errno = C_ERR_ENOENT;
				r = c_err_warn("%s", p->path);
			}
			continue;
		}
		if (IFLAG && yesno(MSG, p->path)) continue;
		switch (p->info) {
		case C_DIR_FSD:
			if (!RFLAG) {
				c_dir_set(&dir, p, C_DIR_FSSKP);
				r = c_err_warnx("%s: %r",
				    p->path, C_ERR_EISDIR);
				continue;
			}
			break;
		case C_DIR_FSDP:
			if (!RFLAG) continue;
			if (c_nix_rmdir(p->path) < 0)
				r = c_err_warn("failed to remove dir \"%s\"",
				    p->path);
			break;
		case C_DIR_FSERR:
			r = c_err_warnx("%s: %r", p->path, p->err);
			break;
		default:
			if ((c_nix_unlink(p->path) < 0) &&
			    (!FFLAG && errno != C_ERR_ENOENT))
				r = c_err_warn("failed to remove \"%s\"",
				    p->path);
		}
	}
	c_dir_close(&dir);
	return r;
}
