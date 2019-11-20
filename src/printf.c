#include <tertium/cpu.h>
#include <tertium/std.h>

#include "common.h"

static uchar
unescape(char *s)
{
	switch (*s) {
	case '\'':
		return '\'';
	case '\\':
		return '\\';
	case 'a':
		return '\a';
	case 'b':
		return '\b';
	case 'f':
		return '\f';
	case 'n':
		return '\n';
	case 'r':
		return '\r';
	case 't':
		return '\t';
	case 'v':
		return '\v';
	}

	return 0;
}

static vlong
stovl(char *s)
{
	vlong x;
	int e;

	if (*s == '\'' || *s == '"') {
		c_utf8_chartorune((ctype_rune *)&x, s + 1);
		return x;
	}

	x = c_std_strtovl(s, 0, C_VLONGMIN, C_VLONGMAX, nil, &e);
	if (e < 0)
		c_err_warn("stovl %s", s);

	return x;
}

static int
numcat(ctype_arr *fmt, char *s, char *argv)
{
	if (*s == '*') {
		edynfmt(fmt, "%d", stovl(argv));
		return 1;
	} else {
		for (; c_str_chr("0123456789", 10, *s); ++s)
			edyncat(fmt, s, 1, sizeof(uchar));
	}
	return 0;
}

static int
printfmt(char *s, int argc, char **argv)
{
	ctype_arr fmt;
	uchar ch;

	c_mem_set(&fmt, sizeof(fmt), 0);
	for (; *s; ++s) {
		ch = *s;
		if (ch == '\\')
			ch = unescape(++s);
		if (ch != '%') {
			c_ioq_nput(ioq1, (char *)&ch, 1);
			continue;
		}
		++s;
		c_arr_trunc(&fmt, 0, sizeof(uchar));
		edyncat(&fmt, "%", 1, sizeof(uchar));
		for (; c_str_chr("#-+", 3, *s); ++s)
			edyncat(&fmt, s, 1, sizeof(uchar));
		if (numcat(&fmt, s, *argv))
			--argc, ++argv;
		if (*s == '.') {
			edyncat(&fmt, s, 1, sizeof(uchar));
			++s;
		}
		if (numcat(&fmt, s, *argv))
			--argc, ++argv;
		switch ((ch = *s)) {
		case 'u':
			edyncat(&fmt, "u", 1, sizeof(uchar));
			/* FALLTHROUGH */
		case 'i':
			ch = 'd';
			/* FALLTHROUGH */
		case 'X':
		case 'd':
		case 'o':
		case 'x':
			if (!argc) {
				c_ioq_nput(ioq1, "0", 1);
				continue;
			}
			edynfmt(&fmt, "ll%c", ch);
			c_ioq_fmt(ioq1, c_arr_data(&fmt), stovl(*argv));
			--argc, ++argv;
			break;
		case 's':
			if (argc) {
				edyncat(&fmt, "s", 1, sizeof(uchar));
				c_ioq_fmt(ioq1, c_arr_data(&fmt), *argv);
				--argc, ++argv;
			}
			break;
		default:
			errno = C_EINVAL;
			c_err_die(1, nil);
		}
	}

	return argc;
}
static void
usage(void)
{
	c_ioq_fmt(ioq2, "usage: %s format [arg ...]\n", c_std_getprogname());
	c_std_exit(1);
}

int
main(int argc, char **argv)
{
	char *fmt;
	int r;

	c_std_setprogname(argv[0]);
	--argc, ++argv;

	if (!argc)
		usage();

	fmt = *argv;
	--argc, ++argv;

	if (!argc) {
		for (; *fmt; ++fmt) {
			char ch = *fmt;
			if (ch == '\\')
				ch = unescape(++fmt);
			c_ioq_nput(ioq1, &ch, 1);
		}
		c_std_exit(0);
	}

	for (;;) {
		if (!(r = printfmt(fmt, argc, argv)))
			break;
		r = argc - r;
		argv += r;
		argc -= r;
	}

	c_ioq_flush(ioq1);
	return 0;
}
