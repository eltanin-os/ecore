#include <tertium/cpu.h>
#include <tertium/std.h>

#include "common.h"

enum {
	AAFLAG = 1 << 0,
	AFLAG  = 1 << 1,
	CFLAG  = 1 << 2,
	DFLAG  = 1 << 3,
	FFFLAG = 1 << 4,
	FFLAG  = 1 << 5,
	GFLAG  = 1 << 6,
	HFLAG  = 1 << 7,
	IFLAG  = 1 << 8,
	LLFLAG = 1 << 9,
	LFLAG  = 1 << 10,
	NFLAG  = 1 << 11,
	OFLAG  = 1 << 12,
	PFLAG  = 1 << 13,
	QFLAG  = 1 << 14,
	RRFLAG = 1 << 15,
	RFLAG  = 1 << 16,
	SSFLAG = 1 << 17,
	SFLAG  = 1 << 18,
	TFLAG  = 1 << 19,
	UFLAG  = 1 << 20,
};

#define TM(a) \
((opts & CFLAG) ? (a)->stp->ctim :\
 (opts & UFLAG) ? (a)->stp->atim : (a)->stp->mtim)

#define TMSEC(a) TM(a).sec
#define TMNSEC(a) TM(a).nsec

struct column {
	size width;
	size num;
};

struct max {
	usize len;
	int block;
	int gid;
	int nlink;
	int ino;
	int size;
	int uid;
	uint btotal;
	uint total;
};

static u32int opts;
static int blksize = 512;
static int first;
static int termwidth = 80;

static int
sort(void *va, void *vb)
{
	ctype_dent *a, *b;
	int cmp;

	a = va;
	b = vb;

	if (opts & SSFLAG) {
		cmp = a->stp->size - b->stp->size;
	} else if (opts & TFLAG) {
		if (!(cmp = TMSEC(a) - TMSEC(b)))
			cmp = TMNSEC(a) - TMNSEC(b);
	} else {
		cmp = c_str_cmp(a->name, C_USIZEMAX, b->name);
	}

	return (opts & RFLAG) ? -cmp : cmp;
}

static int
mkcol(struct column *col, struct max *max)
{
	int twidth;

	col->width = max->len;
	if (opts & IFLAG)
		col->width += max->ino + 1;
	if (opts & SFLAG)
		col->width += max->block + 1;
	if (opts & (FFFLAG | PFLAG))
		col->width += sizeof(uchar);

	++col->width;
	twidth = termwidth + 1;

	if (twidth < (col->width << 1))
		return 1;

	col->num = (twidth / col->width);
	col->width = (twidth / col->num);

	return 0;
}

static int
pwdsize(int type, ctype_fsid id)
{
	char *s;
	int n;

	if ((opts & NFLAG) || !(s = type ? namefromgid(id) : namefromuid(id)))
		return c_std_fmtnil("%llud", (uvlong)id);
	n = c_std_fmtnil("%s", s);
	c_std_free(s);
	return n;
}

static int
noprint(ctype_dent *p)
{
	switch (p->info) {
	case C_FSDOT:
		if (!(opts & AFLAG))
			return 1;
		break;
	case C_FSD:
		if (!(opts & DFLAG) && !p->depth)
			return 1;
	}
	if (*p->name == '.' && !(opts & (AFLAG | AAFLAG)))
		return 1;
	return 0;
}

static void
mkmax(struct max *max, ctype_dir *dir)
{
	ctype_dent *p;
	ctype_fssize block, size;
	ctype_fsid ino, nlink;
	int tmp;

	if (!(p = c_dir_list(dir)))
		return;
	block = size = ino = nlink = tmp = 0;
	++p->parent->num; /* tag this list */
	do {
		if (noprint(p))
			continue;
		++max->total;
		max->btotal += C_HOWMANY(p->stp->blocks, blksize);
		max->len = C_MAX(max->len, p->nlen);
		if (opts & IFLAG)
			ino = C_MAX(ino, (ctype_fsid)p->stp->ino);
		if (opts & SFLAG)
			block = C_MAX(block, (ctype_fssize)p->stp->blocks);
		/* long format */
		if (!(opts & LFLAG))
			continue;
		nlink = C_MAX(nlink, p->stp->nlink);
		size = C_MAX(size, (ctype_fssize)p->stp->size);
		if (opts & OFLAG) {
			tmp = pwdsize(0, p->stp->gid);
			max->gid = C_MAX(max->gid, tmp);
		}
		if (opts & GFLAG) {
			tmp = pwdsize(1, p->stp->uid);
			max->uid = C_MAX(max->uid, tmp);
		}
	} while ((p = c_dir_list(dir)));

	if (opts & IFLAG)
		max->ino   = c_std_fmtnil("%llud", (uvlong)ino);
	if (opts & SFLAG)
		max->block = c_std_fmtnil("%llud", (uvlong)block);
	if (opts & LFLAG) {
		max->nlink = c_std_fmtnil("%llud", (uvlong)nlink);
		max->size  = c_std_fmtnil("%llud", (uvlong)size);
	}
}

static size
pname(ctype_dent *p, int ino, int blk)
{
	ctype_rune rune;
	size n;
	int len;
	char *s;

	n = 0;
	if ((opts & IFLAG) && ino)
		n += c_ioq_fmt(ioq1, "%*llud ", ino, (uvlong)p->stp->ino);
	if ((opts & SFLAG) && blk)
		n += c_ioq_fmt(ioq1, "%*llud ", blk, (uvlong)p->stp->blocks);

	for (s = p->name; *s; s += len) {
		len = c_utf8_chartorune(&rune, s);
		if (!(opts & QFLAG) || 1) {
			(void)c_ioq_nput(ioq1, s, len);
			n += len;
		} else {
			(void)c_ioq_put(ioq1, "?");
			n += sizeof(uchar);
		}
	}

	if ((opts & (FFFLAG | PFLAG)) && C_ISDIR(p->stp->mode)) {
		(void)c_ioq_put(ioq1, "/");
		++n;
	} else if ((opts & FFFLAG)) {
		++n;
		if (C_ISFIFO(p->stp->mode))
			(void)c_ioq_put(ioq1, "|");
		else if (C_ISLNK(p->stp->mode))
			(void)c_ioq_put(ioq1, "@");
		else if (C_ISSCK(p->stp->mode))
			(void)c_ioq_put(ioq1, "=");
		else if (p->stp->mode & (C_IXUSR | C_IXGRP | C_IXOTH))
			(void)c_ioq_put(ioq1, "*");
		else
			--n;
	}
	return n;
}

static void
print1(ctype_dir *dir, struct max *max)
{
	ctype_dent *p;

	if (!(p = c_dir_list(dir)))
		return;
	do {
		if (noprint(p))
			continue;
		if (!(opts & LFLAG)) {
			(void)pname(p, max->ino, max->block);
			(void)c_ioq_put(ioq1, "\n");
			continue;
		}
		/* TODO: LONG FORMAT */
	} while ((p = c_dir_list(dir)));
}

static void
printc(ctype_dir *dir, struct max *max)
{
	struct column cols;
	ctype_dent **pa, *p;
	size chcnt;
	int num, row;
	int base, col, nrows;

	if (mkcol(&cols, max)) {
		print1(dir, max);
		return;
	}

	if (!(pa = c_std_alloc(max->total, sizeof(ctype_dent *))))
		c_err_die(1, "c_std_alloc");

	num = 0;
	if (!(p = c_dir_list(dir)))
		return;
	do {
		if (noprint(p))
			continue;
		if (!(pa[num] = p))
			break;
		++num;
	} while ((p = c_dir_list(dir)));

	nrows = num / cols.num;
	if (num % cols.num)
		++nrows;

	for (row = 0; row < nrows; ++row) {
		for (base = row, col = 0; col < cols.num; ++col) {
			chcnt = pname(pa[base], max->ino, max->block);
			if ((base += nrows) >= num)
				break;
			while (chcnt++ < cols.width)
				(void)c_ioq_put(ioq1, " ");
		}
		(void)c_ioq_put(ioq1, "\n");
	}
	c_std_free(pa);
}

static void
printm(ctype_dir *dir, struct max *max)
{
	ctype_dent *p;
	size chcnt, width;

	width = 0;
	if ((opts & IFLAG))
		width += max->ino;
	if ((opts & SFLAG))
		width += max->block;
	if ((opts & (FFFLAG | PFLAG)))
		++width;

	if (!(p = c_dir_list(dir)))
		return;
	chcnt = 0;
	do {
		if (noprint(p))
			continue;
		if (chcnt > 0) {
			(void)c_ioq_put(ioq1, ",");
			if ((chcnt += 3) + width + (size)p->len >= termwidth) {
				(void)c_ioq_put(ioq1, "\n");
				chcnt = 0;
			} else {
				(void)c_ioq_put(ioq1, " ");
			}
		}
		chcnt += pname(p, max->ino, max->block);
	} while ((p = c_dir_list(dir)));
	(void)c_ioq_put(ioq1, "\n");
}

static void
printx(ctype_dir *dir, struct max *max)
{
	struct column cols;
	ctype_dent *p;
	size chcnt;
	int col;

	if (mkcol(&cols, max)) {
		print1(dir, max);
		return;
	}

	if (!(p = c_dir_list(dir)))
		return;
	col = 0;
	do {
		if (noprint(p))
			continue;
		if (col >= cols.num) {
			col = 0;
			(void)c_ioq_put(ioq1, "\n");
		}
		chcnt = pname(p, max->ino, max->block);
		while (chcnt++ < cols.width)
			(void)c_ioq_put(ioq1, " ");
	} while ((p = c_dir_list(dir)));
	(void)c_ioq_put(ioq1, "\n");
}

static void
usage(void)
{
	c_ioq_fmt(ioq2, "usage: %s [-1AaCcdFfgHikLlmnopqRrSstux] [file ...]\n",
	    c_std_getprogname());
	c_std_exit(1);
}

int
main(int argc, char **argv)
{
	struct max max;
	ctype_dir dir;
	ctype_dent *p;
	int rv;
	uint ropts;
	void (*plist)(ctype_dir *, struct max *);

	c_std_setprogname(argv[0]);

	plist = c_sys_isatty(C_FD1) ? printc : print1;
	ropts = 0;

	C_ARGBEGIN {
	case '1':
		plist = &print1;
		break;
	case 'A':
		opts = (opts & ~AFLAG) | AAFLAG;
		break;
	case 'C':
		plist = &printc;
		break;
	case 'F':
		opts = (opts & ~PFLAG) | FFFLAG;
		break;
	case 'H':
		ropts |= C_FSCOM;
		break;
	case 'L':
		ropts = (ropts & ~C_FSPHY) | C_FSLOG;
		break;
	case 'R':
		opts = (opts & ~DFLAG) | RRFLAG;
		break;
	case 'S':
		opts = (opts & ~(FFLAG | TFLAG)) | SSFLAG;
		break;
	case 'a':
		opts = (opts & ~AAFLAG) | AFLAG;
		break;
	case 'c':
		opts = (opts & ~UFLAG) | CFLAG;
		break;
	case 'd':
		opts = (opts & ~RRFLAG) | DFLAG;
		break;
	case 'f':
		opts = (opts & ~(TFLAG | SSFLAG)) | FFLAG;
		break;
	case 'g':
		plist = &print1;
		opts |= GFLAG | LFLAG;
		break;
	case 'i':
		opts |= IFLAG;
		break;
	case 'k':
		blksize = 1024;
		break;
	case 'l':
		opts |= LFLAG;
		break;
	case 'm':
		plist = &printm;
		break;
	case 'n':
		plist = &print1;
		opts |= LFLAG | NFLAG;
		break;
	case 'o':
		plist = &print1;
		opts |= LFLAG | OFLAG;
		break;
	case 'p':
		opts |= PFLAG;
		break;
	case 'q':
		opts |= QFLAG;
		break;
	case 'r':
		opts |= RFLAG;
		break;
	case 's':
		opts |= SFLAG;
		break;
	case 't':
		opts = (opts & ~(FFLAG | SSFLAG)) | TFLAG;
		break;
	case 'u':
		opts = (opts & ~CFLAG) | UFLAG;
	case 'x':
		plist = printx;
		break;
	default:
		usage();
	} C_ARGEND

	if (!argc)
		argv = tmpargv(".");

	if (c_dir_open(&dir, argv, ropts, (opts & FFLAG) ? nil : &sort) < 0)
		c_err_die(1, "c_dir_open");

	(void)c_mem_set(&max, sizeof(max), 0);
	mkmax(&max, &dir);
	plist(&dir, &max);

	if (max.total)
		first = 1;

	rv = 0;

	while ((p = c_dir_read(&dir))) {
		switch (p->info) {
		case C_FSD:
			if (!(opts & RRFLAG) && ((opts & DFLAG) || p->depth)) {
				c_dir_set(&dir, p, C_FSSKP);
				continue;
			}
			if (p->depth || argc > 1)
				c_ioq_fmt(ioq1, "%s%s:\n",
				    first ? "\n" : (first++, ""), p->path);
			break;
		case C_FSDNR:
		case C_FSNS:
		case C_FSERR:
			rv = c_err_warnx("%s: %s", p->path, serr(p->err));
			break;
		}

		if (p->parent->num)
			continue;

		(void)c_mem_set(&max, sizeof(max), 0);
		mkmax(&max, &dir);
		plist(&dir, &max);
	}
	(void)c_ioq_flush(ioq1);
	return 0;
}
