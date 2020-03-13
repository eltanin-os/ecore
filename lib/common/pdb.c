#include <tertium/cpu.h>
#include <tertium/std.h>

#include "common.h"

ctype_status
dbgetln(struct pdb *p, ctype_ioq *ioq, ctype_arr *arr)
{
	ctype_status r;
	int i;
	char *s;

	c_arr_trunc(arr, 0, sizeof(uchar));
	if ((r = c_ioq_getln(ioq, arr)) <= 0)
		return r;

	s = p->p[0] = c_arr_data(arr);
	i = 1;
	while ((s = c_str_chr(s, C_USIZEMAX, ':'))) {
		*s++ = 0;
		p->p[i++] = s;
	}
	return 1;
}

static ctype_status
readfile(struct pdb *p, char *file, char *search, int pos)
{
	ctype_arr arr;
	ctype_ioq ioq;
	ctype_fd fd;
	char buf[C_BIOSIZ];

	if ((fd = c_sys_open(file, C_OREAD, 0)) < 0)
		c_err_die(1, "c_sys_open %s", file);

	c_ioq_init(&ioq, fd, buf, sizeof(buf), &c_sys_read);
	c_mem_set(&arr, sizeof(arr), 0);
	while (dbgetln(p, &ioq, &arr) > 0) {
		if (c_str_cmp(p->p[pos], c_arr_bytes(&arr), search))
			continue;
		c_sys_close(fd);
		return 0;
	}
	c_sys_close(fd);
	return -1;
}

static ctype_status
searchfromid(struct pdb *p, char *file, ctype_id id, int pos)
{
	ctype_arr arr;
	char buf[16];

	c_arr_init(&arr, buf, sizeof(buf));
	c_arr_fmt(&arr, "%ud", id);
	return readfile(p, file, buf, pos);
}

static ctype_id
idfromname(char *file, char *name)
{
	struct pdb db;
	ctype_id x;

	if (readfile(&db, file, name, 0) < 0)
		return -1;
	x = c_std_strtovl(db.p[2], 0, 0, C_UINTMAX, nil, nil);
	c_std_free(*db.p);
	return x;
}

ctype_id
gidfromuid(ctype_id uid)
{
	struct pdb db;
	ctype_id x;

	if (searchfromid(&db, PWDFILE, uid, 2) < 0)
		return -1;
	x = estrtovl(db.p[3], 0, 0, C_UINTMAX);
	c_std_free(*db.p);
	return x;
}

char *
namefromuid(ctype_id uid)
{
	struct pdb db;

	if (searchfromid(&db, PWDFILE, uid, 2) < 0)
		return nil;
	return db.p[0];
}

char *
namefromgid(ctype_id gid)
{
	struct pdb db;

	if (searchfromid(&db, GRPFILE, gid, 2) < 0)
		return nil;
	return db.p[0];
}

ctype_id
uidfromname(char *name)
{
	return idfromname(PWDFILE, name);
}

ctype_id
gidfromname(char *name)
{
	return idfromname(GRPFILE, name);
}
