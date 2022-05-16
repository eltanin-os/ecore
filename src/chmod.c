#include <tertium/cpu.h>
#include <tertium/std.h>

#include "common.h"

static void
usage(void)
{
	c_ioq_fmt(ioq2, "usage: %s [-R] mode file ...\n", c_std_getprogname());
	c_std_exit(1);
}

ctype_status
main(int argc, char **argv)
{
	ctype_dir dir;
	ctype_dent *p;
	ctype_status r;
	int Rflag;
	uint mask;
	char *m;

	c_std_setprogname(argv[0]);
	--argc, ++argv;

	Rflag = 0;

	while (c_std_getopt(argmain, argc, argv, "RXrstxw")) {
		switch (argmain->opt) {
		case 'R':
			Rflag = 1;
			break;
		case 'X':
		case 'r':
		case 's':
		case 't':
		case 'x':
		case 'w':
			--argmain->idx;
			goto done;
		default:
			usage();
		}
	}
done:
	argc -= argmain->idx;
	argv += argmain->idx;
	if (argc < 2) usage();

	mask = c_nix_getumask();
	m = *argv;
	--argc, ++argv;

	if (c_dir_open(&dir, argv, 0, nil) < 0) c_err_die(1, "c_dir_open");
	r = 0;
	while ((p = c_dir_read(&dir))) {
		switch (p->info) {
		case C_DIR_FSD:
			if (!Rflag)
				c_dir_set(&dir, p, C_DIR_FSSKP);
			break;
		case C_DIR_FSDNR:
			r = c_err_warnx("%s: %r", p->name, p->err);
			continue;
		case C_DIR_FSDP:
			continue;
		case C_DIR_FSERR:
		case C_DIR_FSNS:
			r = c_err_warnx("%s: %r", p->name, p->err);
			continue;
		case C_DIR_FSSL:
		case C_DIR_FSSLN:
			continue;
		}

		if (c_nix_chmod(p->path,
		    c_nix_strtomode(m, p->stp->mode, mask)) < 0)
			r = c_err_warn("c_nix_chmod %s", p->path);
	}
	c_dir_close(&dir);
	return r;
}
