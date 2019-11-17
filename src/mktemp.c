#include <tertium/cpu.h>
#include <tertium/std.h>

#include "common.h"

#define DIE(...) \
{ if (!(opts & QFLAG)) c_err_warn(__VA_ARGS__); c_std_exit(1); }
#define DIEX(...) \
{ if (!(opts & QFLAG)) c_err_warnx(__VA_ARGS__); c_std_exit(1); }

enum {
	DFLAG = 1 << 0,
	TFLAG = 1 << 1,
	QFLAG = 1 << 2,
	UFLAG = 1 << 3,
};

static void
usage(void)
{
	c_ioq_fmt(ioq2, "usage: %s [-dqtu] [-p directory] [template]\n",
	    c_std_getprogname());
	c_std_exit(1);
}

int
main(int argc, char **argv)
{
	ctype_arr arr;
	int fd;
	uint n, opts;
	char *dir, *template, *tmp;
	char buf[C_PATHMAX];

	c_std_setprogname(argv[0]);

	dir = nil;
	opts = 0;

	C_ARGBEGIN {
	case 'd':
		opts |= DFLAG;
		break;
	case 'p':
		opts |= TFLAG;
		dir = C_EARGF(usage());
		break;
	case 'q':
		opts |= QFLAG;
		break;
	case 't':
		opts |= TFLAG;
		break;
	case 'u':
		opts |= UFLAG;
		break;
	default:
		usage();
	} C_ARGEND

	switch (argc) {
	case 0:
		template = "tmp.XXXXXXXXXX";
		opts |= TFLAG;
		break;
	case 1:
		template = *argv;
		break;
	default:
		template = nil;
		usage();
	}

	c_arr_init(&arr, buf, sizeof(buf));

	if (!argc || (opts & TFLAG)) {
		if (c_str_chr(template, C_USIZEMAX, '/'))
			DIEX("Template must not contain directory "
			    "separators in -t mode");
		if (!(tmp = c_sys_getenv("TMPDIR"))) {
			if (dir)
				tmp = dir;
			else
				tmp = "/tmp";
		}
		trim_trailing_slash(tmp);
		if (c_arr_fmt(&arr, "%s/", tmp) < 0)
			DIE("c_arr_fmt");
	}

	if (c_arr_fmt(&arr, "%s", template) < 0)
		DIE("c_arr_fmt");

	tmp = buf + c_arr_bytes(&arr) - 1;
	for (n = 0; *tmp == 'X'; --tmp, ++n) ;

	if (!n)
		DIEX("Invalid template");

	++tmp;
	++n;
	for (;;) {
		c_rand_name(tmp, n);
		if (opts & DFLAG) {
			if (c_sys_mkdir(c_arr_data(&arr), 0700) < 0) {
				if (errno != C_EEXIST)
					DIE("c_sys_mkdir %s", c_arr_data(&arr));
				continue;
			}
			if (opts & UFLAG)
				c_sys_rmdir(c_arr_data(&arr));
		} else {
			if ((fd = c_sys_open(c_arr_data(&arr), C_OCREATE, 0600)) < 0) {
				if (errno != C_EEXIST)
					DIE("c_sys_open %s", c_arr_data(&arr));
				continue;
			}
			c_sys_close(fd);
			if (opts & UFLAG)
				c_sys_unlink(c_arr_data(&arr));
		}
		break;
	}

	c_ioq_fmt(ioq1, "%s\n", c_arr_data(&arr));
	c_ioq_flush(ioq1);
	return 0;
}
