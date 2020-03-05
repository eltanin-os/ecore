#include <tertium/cpu.h>
#include <tertium/std.h>

#include "common.h"

#define PWDFILE "/etc/passwd"
#define GRPFILE "/etc/group"

/* pwd: name:uid:gid:dir:shell
 * grp: name:gid:mem */
struct pdb {
	char *p[5];
};

static ctype_status
readfile(struct pdb *p, char *file, char *search)
{
	ctype_arr arr;
	ctype_ioq ioq;
	ctype_fd fd;
	int i;
	char buf[C_BIOSIZ];
	char *s;

	if ((fd = c_sys_open(file, C_OREAD, 0)) < 0)
		c_err_die(1, "c_sys_open %s", file);

	c_ioq_init(&ioq, fd, buf, sizeof(buf), &c_sys_read);
	c_mem_set(&arr, sizeof(arr), 0);
	while (c_ioq_getln(&ioq, &arr) > 0) {
		p->p[0] = s = c_arr_data(&arr);
		if (!(c_str_str(s, c_arr_bytes(&arr), search))) {
			c_arr_trunc(&arr, 0, sizeof(uchar));
			continue;
		}
		i = 1;
		while ((s = c_mem_chr(s, c_arr_bytes(&arr), ':'))) {
			*s++ = 0;
			p->p[i++] = s;
		}
		return 0;
	}
	c_dyn_free(&arr);
	return -1;
}

static char *
readid(char *file, ctype_id id)
{
	struct pdb db;
	ctype_arr arr;
	char buf[32];

	c_arr_init(&arr, buf, sizeof(buf));
	c_arr_fmt(&arr, ":%ud:", id);
	return (readfile(&db, file, buf) < 0) ? nil : db.p[0];
}

char *
namefromuid(ctype_id uid)
{
	return readid(PWDFILE, uid);
}

char *
namefromgid(ctype_id gid)
{
	return readid(GRPFILE, gid);
}

static ctype_id
readname(char *file, char *name)
{
	struct pdb db;
	ctype_id x;

	if (readfile(&db, file, name) < 0)
		return -1;
	x = c_std_strtovl(db.p[2], 0, 0, C_UINTMAX, nil, nil);
	c_std_free(*db.p);
	return x;
}

ctype_id
uidfromname(char *name)
{
	return readname(PWDFILE, name);
}

ctype_id
gidfromname(char *name)
{
	return readname(GRPFILE, name);
}
