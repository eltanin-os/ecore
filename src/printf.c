#include <tertium/cpu.h>
#include <tertium/std.h>

#include "common.h"

#define IS_ODIGIT(x) ((x) >= '0' && (x) <= '7')

#define NUMCAT(a, b, c) \
{ if ((r = numcat((a), (b), (c))) < 0) --argc, ++argv, ++s; else s += r; }

static void
unescape(char **s)
{
	int ch, i;
	char tab[] = "\\\t \v \a\b   \f     \' \n   \r";

	++*s;
	if (IS_ODIGIT(*s[0])) {
		for (ch = i = 0; i < 4 && IS_ODIGIT(*s[0]); ++i) {
			ch = ch << 3 | (*s[0] - '0');
			++*s;
		}
		--*s;
		c_ioq_nput(ioq1, (char *)&ch, 1);
		return;
	}

	if (*s[0] == 'c')
		c_std_exit(0);

	switch (*s[0]) {
	case 'a':
	case 'b':
	case 't':
	case 'n':
	case 'v':
	case 'f':
	case 'r':
	case '\'':
	case '\\':
		c_ioq_nput(ioq1, tab + (*s[0] % (sizeof(tab) - 1)), 1);
	}
}

static vlong
stovl(char *s)
{
	ctype_rune r;
	vlong x;
	int e;

	if (*s == '\'' || *s == '"') {
		c_utf8_chartorune(&r, s + 1);
		return (vlong)r;
	}

	x = c_std_strtovl(s, 0, C_VLONGMIN, C_VLONGMAX, nil, &e);
	if (e < 0)
		c_err_warn("stovl %s", s);

	return x;
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
	ctype_arr fmt;
	int ac, r;
	uchar ch;

	ac = argc;
	c_mem_set(&fmt, sizeof(fmt), 0);
	for (; *s; ++s) {
		ch = *s;
		if (ch == '\\') {
			unescape(&s);
			continue;
		} else if (ch != '%') {
			c_ioq_nput(ioq1, (char *)&ch, 1);
			continue;
		}
		++s;
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

	return ac - argc;
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

	do {
		if (!(r = printfmt(fmt, argc, argv)))
			break;
		argv += r;
		argc -= r;
	} while(argc);

	c_ioq_flush(ioq1);
	return 0;
}
