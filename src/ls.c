#include <tertium/cpu.h>
#include <tertium/std.h>

#include "common.h"

enum {
	AFLAG  = 1 << 0,
	CFLAG  = 1 << 1,
	DFLAG  = 1 << 2,
	FFFLAG = 1 << 3,
	FFLAG  = 1 << 4,
	GFLAG  = 1 << 5,
	HFLAG  = 1 << 6,
	IFLAG  = 1 << 7,
	LLFLAG = 1 << 8,
	LFLAG  = 1 << 9,
	NFLAG  = 1 << 10,
	OFLAG  = 1 << 11,
	PFLAG  = 1 << 12,
	QFLAG  = 1 << 13,
	RRFLAG = 1 << 14,
	RFLAG  = 1 << 15,
	SSFLAG = 1 << 16,
	SFLAG  = 1 << 17,
	TFLAG  = 1 << 18,
	UFLAG  = 1 << 19,
};

#define STPFLAG (FFFLAG | GFLAG | IFLAG | OFLAG | PFLAG | SFLAG | TFLAG)

#define SECSPERDAY (24 * 60 * 60)
#define SIXMONTHS  (180 * SECSPERDAY)

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

static char *mtab[] = {
	"Jan", "Feb", "Mar", "Apr", "May", "Jun",
	"Jul", "Aug", "Sep", "Oct", "Nov", "Dec",
};

static u64 now;
static u32 opts;
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
		cmp = b->stp->size - a->stp->size;
	} else if (opts & TFLAG) {
		if (!(cmp = TMSEC(b) - TMSEC(a)))
			cmp = TMNSEC(b) - TMNSEC(a);
	} else {
		cmp = c_str_cmp(a->name, -1, b->name);
	}
	return (opts & RFLAG) ? -cmp : cmp;
}

static int
mkcol(struct column *col, struct max *max)
{
	int twidth;

	col->width = max->len;
	if (opts & IFLAG) col->width += max->ino + 1;
	if (opts & SFLAG) col->width += max->block + 1;
	if (opts & (FFFLAG | PFLAG)) col->width += sizeof(uchar);

	++col->width;
	twidth = termwidth + 1;
	if (twidth < (col->width << 1)) return 1;

	col->num = (twidth / col->width);
	col->width = (twidth / col->num);
	return 0;
}

static void
printid(int type, ctype_fsid id, usize max)
{
	char *s;

	if ((opts & NFLAG) || !(s = type ? namefromgid(id) : namefromuid(id))) {
		c_ioq_fmt(ioq1, "%-*llud ", max, (uvlong)id);
	} else {
		c_ioq_fmt(ioq1, "%-*s ", max, s);
		c_std_free(s);
	}

}

static int
idsize(int type, ctype_fsid id)
{
	char *s;
	int n;

	if ((opts & NFLAG) || !(s = type ? namefromgid(id) : namefromuid(id)))
		return c_str_fmtcnt("%llud", (uvlong)id);
	n = c_str_fmtcnt("%s", s);
	c_std_free(s);
	return n;
}

static int
noprint(ctype_dent *p)
{
	switch (p->info) {
	case C_DIR_FSD:
		if (!(opts & DFLAG) && !p->depth) return 1;
		break;
	case C_DIR_FSDNR:
	case C_DIR_FSNS:
	case C_DIR_FSERR:
		return 1;
	}
	if (*p->name == '.' && !(opts & AFLAG)) return 1;
	return 0;
}

static void
mkmax(struct max *max, ctype_dir *dir)
{
	ctype_dent *p;
	ctype_fssize block, size;
	ctype_fsid ino, nlink;
	int tmp;

	if (!(p = c_dir_list(dir))) return;
	block = size = ino = nlink = tmp = 0;
	++p->parent->num; /* tag this list */
	do {
		if (noprint(p)) continue;
		++max->total;
		max->len = C_STD_MAX(max->len, p->nlen);
		if (opts & IFLAG)
			ino = C_STD_MAX(ino, (ctype_fsid)p->stp->ino);
		if (opts & SFLAG)
			block = C_STD_MAX(block, (ctype_fssize)p->stp->blocks);
		if (opts & (GFLAG | IFLAG | LFLAG | OFLAG | SFLAG))
			max->btotal += C_STD_HOWMANY(p->stp->blocks, blksize);
		/* long format */
		if (!(opts & LFLAG)) continue;
		nlink = C_STD_MAX(nlink, p->stp->nlink);
		size = C_STD_MAX(size, (ctype_fssize)p->stp->size);
		if (opts & OFLAG) {
			tmp = idsize(0, p->stp->gid);
			max->gid = C_STD_MAX(max->gid, tmp);
		}
		if (opts & GFLAG) {
			tmp = idsize(1, p->stp->uid);
			max->uid = C_STD_MAX(max->uid, tmp);
		}
	} while ((p = c_dir_list(dir)));

	if (opts & IFLAG) max->ino = c_str_fmtcnt("%llud", (uvlong)ino);
	if (opts & SFLAG) max->block = c_str_fmtcnt("%llud", (uvlong)block);
	if (opts & LFLAG) {
		max->nlink = c_str_fmtcnt("%llud", (uvlong)nlink);
		max->size = c_str_fmtcnt("%llud", (uvlong)size);
	}
}

static int
printtype(ctype_stat *st)
{
	if ((opts & (FFFLAG | PFLAG)) && C_NIX_ISDIR(st->mode)) {
		return c_ioq_fmt(ioq1, "/");
	} else if ((opts & FFFLAG)) {
		if (C_NIX_ISFIFO(st->mode))
			return c_ioq_fmt(ioq1, "|");
		else if (C_NIX_ISLNK(st->mode))
			return c_ioq_fmt(ioq1, "@");
		else if (C_NIX_ISSCK(st->mode))
			return c_ioq_fmt(ioq1, "=");
		else if (st->mode & (C_NIX_IXUSR | C_NIX_IXGRP | C_NIX_IXOTH))
			return c_ioq_fmt(ioq1, "*");
	}
	return 0;
}

static size
printname(ctype_dent *p, int ino, int blk)
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

	for (s = p->depth ? p->name : p->path; *s; s += len) {
		len = c_utf8_chartorune(&rune, s);
		if (!(opts & QFLAG) || c_utf8_isprint(rune))
			c_ioq_nput(ioq1, s, len);
		else
			c_ioq_put(ioq1, "?");
		++n;
	}
	n += printtype(p->stp);
	return n;
}

static void
printmode(ctype_stat *p)
{
	uint m;
	char mode[11];
	char *s;

	c_mem_cpy(mode, sizeof(mode), "?---------");
	switch (p->mode & C_NIX_IFMT) {
	case C_NIX_IFBLK:
		mode[0] = 'b';
		break;
	case C_NIX_IFCHR:
		mode[0] = 'c';
		break;
	case C_NIX_IFDIR:
		mode[0] = 'd';
		break;
	case C_NIX_IFIFO:
		mode[0] = 'p';
		break;
	case C_NIX_IFLNK:
		mode[0] = 'l';
		break;
	case C_NIX_IFREG:
		mode[0] = '-';
		break;
	}

	s = mode + 1;
	m = p->mode;
	do {
		if (m & C_NIX_IRUSR) s[0] = 'r';
		if (m & C_NIX_IWUSR) s[1] = 'w';
		if (m & C_NIX_IXUSR) s[2] = 'x';
		m <<= 3;
	} while ((s += 3) <= mode + 7);

	if (p->mode & C_NIX_ISUID) mode[3] = (mode[3] == 'x') ? 's' : 'S';
	if (p->mode & C_NIX_ISGID) mode[6] = (mode[6] == 'x') ? 's' : 'S';
	if (p->mode & C_NIX_ISVTX) mode[9] = (mode[9] == 'x') ? 't' : 'T';

	c_ioq_fmt(ioq1, "%s ", mode);
}

static void
printtime(ctype_dent *p)
{
	ctype_caltime ct;
	ctype_tai t;
	ctype_time tm;

	tm = TM(p);
	c_tai_fromtime(&t, &tm);
	c_cal_timeutc(&ct, &t);
	if (now > (c_tai_approx(&t) + SIXMONTHS)) {
		c_ioq_fmt(ioq1, "%s %02d  %04ld ",
		    mtab[ct.date.month - 1], ct.date.day, ct.date.year);
	} else {
		c_ioq_fmt(ioq1, "%s %02d %02d:%02d ",
		    mtab[ct.date.month - 1], ct.date.day, ct.hour, ct.minute);
	}
}

static void
printlink(char *s)
{
	ctype_stat st;
	size r;
	char buf[C_LIM_PATHMAX];

	if ((r = c_nix_readlink(buf, sizeof(buf), s)) < 0) {
		c_err_warn("readlink %s", s);
		return;
	}
	c_ioq_fmt(ioq1, " -> %s", buf);
	if (!c_nix_stat(&st, s)) printtype(&st);
}

static void
print1(ctype_dir *dir, struct max *max)
{
	ctype_dent *p;

	if (!(p = c_dir_list(dir))) return;
	do {
		if (noprint(p)) continue;
		if (!(opts & LFLAG)) {
			printname(p, max->ino, max->block);
			c_ioq_put(ioq1, "\n");
			continue;
		}
		if (opts & IFLAG)
			c_ioq_fmt(ioq1, "%*llud ",
			    max->ino, (uvlong)p->stp->ino);
		if (opts & SFLAG)
			c_ioq_fmt(ioq1, "%*llud ",
			    max->block, (uvlong)p->stp->blocks);
		printmode(p->stp);
		c_ioq_fmt(ioq1, "%*lud ", max->nlink, p->stp->nlink);
		if (!(opts & GFLAG)) printid(0, p->stp->uid, max->uid);
		if (!(opts & OFLAG)) printid(1, p->stp->gid, max->gid);
		if (C_NIX_ISBLK(p->stp->mode) || C_NIX_ISCHR(p->stp->mode))
			c_ioq_fmt(ioq1, "%3d, %3d ",
			    C_NIX_MAJOR(p->stp->rdev),
			    C_NIX_MINOR(p->stp->rdev));
		else
			c_ioq_fmt(ioq1, "%*s%*lld ",
			    8 - max->size, "", max->size, (vlong)p->stp->size);
		printtime(p);
		printname(p, max->ino, max->block);
		if (C_NIX_ISLNK(p->stp->mode)) printlink(p->path);
		c_ioq_put(ioq1, "\n");
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
		c_err_diex(1, nil);

	num = 0;
	if (!(p = c_dir_list(dir))) return;
	do {
		if (noprint(p)) continue;
		if (!(pa[num] = p)) break;
		++num;
	} while ((p = c_dir_list(dir)));

	nrows = num / cols.num;
	if (num % cols.num) ++nrows;

	for (row = 0; row < nrows; ++row) {
		for (base = row, col = 0; col < cols.num; ++col) {
			chcnt = printname(pa[base], max->ino, max->block);
			if ((base += nrows) >= num) break;
			c_ioq_fmt(ioq1, "%*s", cols.width - chcnt, "");
		}
		c_ioq_put(ioq1, "\n");
	}
	c_std_free(pa);
}

static void
printm(ctype_dir *dir, struct max *max)
{
	ctype_dent *p;
	size chcnt, width;

	width = 0;
	if ((opts & IFLAG)) width += max->ino;
	if ((opts & SFLAG)) width += max->block;
	if ((opts & (FFFLAG | PFLAG))) ++width;

	if (!(p = c_dir_list(dir))) return;
	chcnt = 0;
	do {
		if (noprint(p)) continue;
		if (chcnt > 0) {
			c_ioq_put(ioq1, ",");
			if ((chcnt += 3) + width + (size)p->len >= termwidth) {
				c_ioq_put(ioq1, "\n");
				chcnt = 0;
			} else {
				c_ioq_put(ioq1, " ");
			}
		}
		chcnt += printname(p, max->ino, max->block);
	} while ((p = c_dir_list(dir)));
	if (chcnt) c_ioq_put(ioq1, "\n");
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

	if (!(p = c_dir_list(dir))) return;
	chcnt = col = 0;
	do {
		if (noprint(p)) continue;
		if (col >= cols.num) {
			col = 0;
			c_ioq_put(ioq1, "\n");
		}
		chcnt = printname(p, max->ino, max->block);
		c_ioq_fmt(ioq1, "%*s", cols.width - chcnt, "");
		++col;
	} while ((p = c_dir_list(dir)));
	if (chcnt) c_ioq_put(ioq1, "\n");
}

static int
ttycheck(ctype_fd fd)
{
	ctype_stat st;
	ctype_status r;
	r = c_nix_fdstat(&st, fd);
	if (r < 0) c_err_die(1, "failed to obtain tty info");
	return C_NIX_ISCHR(st.mode);
}

static void
usage(void)
{
	c_ioq_fmt(ioq2, "usage: %s [-1AaCcdFfgHikLlmnopqRrSstux] [file ...]\n",
	    c_std_getprogname());
	c_std_exit(1);
}

ctype_status
main(int argc, char **argv)
{
	struct max max;
	ctype_dir dir;
	ctype_dent *p;
	ctype_tai t;
	ctype_status r;
	uint ropts;
	char *tmp;
	void (*plist)(ctype_dir *, struct max *);

	c_std_setprogname(argv[0]);
	--argc, ++argv;

	plist = ttycheck(C_IOQ_FD1) ? printc : print1;
	ropts = 0;

	while (c_std_getopt(argmain, argc, argv,
	    "1ACFHLRSacdfgiklmnopqrstux")) {
		switch (argmain->opt) {
		case '1':
			plist = &print1;
			break;
		case 'A':
			opts |= AFLAG;
			ropts &= ~C_DIR_FSVDT;
			break;
		case 'C':
			plist = &printc;
			break;
		case 'F':
			opts = (opts & ~PFLAG) | FFFLAG;
			break;
		case 'H':
			ropts |= C_DIR_FSCOM;
			break;
		case 'L':
			ropts = (ropts & ~C_DIR_FSPHY) | C_DIR_FSLOG;
			break;
		case 'R':
			opts = (opts & ~DFLAG) | RRFLAG;
			break;
		case 'S':
			opts = (opts & ~(FFLAG | TFLAG)) | SSFLAG;
			break;
		case 'a':
			opts |= AFLAG;
			ropts |= C_DIR_FSVDT;
			break;
		case 'c':
			opts = (opts & ~UFLAG) | CFLAG;
			break;
		case 'd':
			opts = (opts & ~RRFLAG) | DFLAG;
			break;
		case 'f':
			opts = (opts & ~(TFLAG | SSFLAG)) | AFLAG | FFLAG;
			ropts |= C_DIR_FSVDT;
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
			plist = &print1;
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
			break;
		case 'x':
			plist = printx;
			break;
		default:
			usage();
		}
	}
	argc -= argmain->idx;
	argv += argmain->idx;
	if (!argc) argv = tmpargv(".");

	if (opts & LFLAG) {
		c_tai_now(&t);
		now = c_tai_approx(&t);
	} else if (!(opts & STPFLAG)) {
		ropts |= C_DIR_FSNOI;
	}

	if (c_dir_open(&dir, argv, ropts, (opts & FFLAG) ? nil : &sort) < 0)
		c_err_die(1, nil);

	blksize /= 512;
	if ((tmp = c_std_getenv("COLUMNS")))
		termwidth = estrtovl(tmp, 0, 0, C_LIM_UINTMAX);

	c_mem_set(&max, sizeof(max), 0);
	mkmax(&max, &dir);
	plist(&dir, &max);
	if (max.total) first = 1;

	r = 0;
	while ((p = c_dir_read(&dir))) {
		switch (p->info) {
		case C_DIR_FSD:
			if ((!(opts & RRFLAG) &&
			    ((opts & DFLAG) || p->depth)) ||
			    (p->depth && (p->name[0] == '.' && p->name[1] &&
			    !(opts & AFLAG)))) {
				c_dir_set(&dir, p, C_DIR_FSSKP);
				continue;
			}
			if (opts & RRFLAG || p->depth || argc > 1)
				c_ioq_fmt(ioq1, "%s%s:\n",
				    first ? "\n" : (first++, ""), p->path);
			break;
		case C_DIR_FSDNR:
		case C_DIR_FSNS:
		case C_DIR_FSERR:
			r = c_err_warnx("%s: %r", p->path, p->err);
			break;
		}

		if (p->parent->num) continue;

		c_mem_set(&max, sizeof(max), 0);
		mkmax(&max, &dir);

		if (opts & (GFLAG | LFLAG | OFLAG | SFLAG))
			c_ioq_fmt(ioq1, "total %ud\n", max.btotal);

		plist(&dir, &max);
	}
	c_dir_close(&dir);
	c_ioq_flush(ioq1);
	return r;
}
