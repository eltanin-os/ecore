#include <tertium/cpu.h>
#include <tertium/std.h>

#include "common.h"

#define ODIGIT(x) ((x) >= '0' && (x) <= '7')
#define NUMCAT(a, b, c) \
{ int r; s += (r = numcat((a), (b), (c)) < 0) ? (--argc, ++argv, 1) : r; }

static void
unescape(char **s)
{
	int ch, i;

	++*s;
	if (ODIGIT(*s[0])) {
		for (ch = i = 0; i < 4 && ODIGIT(*s[0]); ++i) {
			ch = ch << 3 | (*s[0] - '0');
			++*s;
		}
		--*s;
		c_ioq_nput(ioq1, (char *)&ch, 1);
		return;
	}

	switch (*s[0]) {
	case 'a':
		ch = '\a';
		break;
	case 'b':
		ch = '\b';
		break;
	case 'c':
		c_std_exit(0);
	case 't':
		ch = '\t';
		break;
	case 'n':
		ch = '\n';
		break;
	case 'v':
		ch = '\v';
		break;
	case 'f':
		ch = '\f';
		break;
	case 'r':
		ch = '\r';
		break;
	case '\'':
		ch = '\'';
		break;
	case '\\':
		ch = '\\';
	}
	c_ioq_nput(ioq1, (char *)&ch, 1);
}

static vlong
stovl(char *s)
{
	ctype_rune r;

	if (*s == '\'' || *s == '"') {
		c_utf8_chartorune(&r, s + 1);
		return (vlong)r;
	}
	return estrtovl(s, 0, C_LIM_VLONGMIN, C_LIM_VLONGMAX);
}

static int
numcat(ctype_arr *fmt, char *s, char *argv)
{
	char *p;

	p = s;
	if (*p == '*') {
		edynfmt(fmt, "%d", stovl(argv));
		return -1;
	} else {
		for (; c_str_chr("0123456789", 10, *p); ++p)
			edyncat(fmt, p, 1, sizeof(uchar));
	}
	return p - s;
}

static int
printfmt(char *s, int argc, char **argv)
{
	static ctype_arr fmt; /* "memory leak" */
	ctype_rune rune;
	int ac;
	uchar ch;

	ac = argc;
	c_arr_trunc(&fmt, 0, sizeof(uchar));
	for (; *s; ++s) {
		ch = *s;
		switch (ch) {
		case '\\':
			unescape(&s);
			continue;
		case '%':
			++s;
			if (*s != '%') break;
			/* FALLTHROUGH */
		default:
			c_ioq_nput(ioq1, (char *)&ch, 1);
			continue;
		}
		c_arr_trunc(&fmt, 0, sizeof(uchar));
		edyncat(&fmt, "%", 1, sizeof(uchar));
		for (; c_str_chr("#-+", 3, *s); ++s)
			edyncat(&fmt, s, 1, sizeof(uchar));
		NUMCAT(&fmt, s, *argv);
		if (*s == '.') {
			edyncat(&fmt, s, 1, sizeof(uchar));
			++s;
		}
		NUMCAT(&fmt, s, *argv);
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
		case 'c':
			argv[0][c_utf8_chartorune(&rune, *argv)] = 0;
			/* FALLTHROUGH */
		case 's':
			if (argc) {
				edyncat(&fmt, "s", 1, sizeof(uchar));
				c_ioq_fmt(ioq1, c_arr_data(&fmt), *argv);
				--argc, ++argv;
			}
			break;
		default:
			errno = C_ERR_EINVAL;
			c_err_die(1, "%%%c", ch);
		}
	}
	return ac - argc;
}

static void
usage(void)
{
	c_ioq_fmt(ioq2, "usage: %s format [arg ...]\n", c_std_getprogname());
	c_std_exit(1);
}

ctype_status
main(int argc, char **argv)
{
	char *fmt;
	int r;

	c_std_setprogname(argv[0]);
	--argc, ++argv;

	if (c_std_noopt(argmain, *argv)) usage();
	argc -= argmain->idx;
	if (!argc) usage();
	argv += argmain->idx;

	fmt = *argv;
	--argc, ++argv;
	do {
		if (!(r = printfmt(fmt, argc, argv))) break;
		argv += r;
		argc -= r;
	} while(argc);
	c_ioq_flush(ioq1);
	return 0;
}
