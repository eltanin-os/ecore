enum {
	/* cp flags */
	CP_FFLAG = 1 << 0,
	CP_PFLAG = 1 << 1,
	CP_RFLAG = 1 << 2,
	CP_IFLAG = 1 << 3,
	CP_TDIR = 1 << 4,
	CP_ATOMIC = 1 << 5,

	/* rm flags */
	RM_FFLAG = 1 << 0,
	RM_RFLAG = 1 << 1,
	RM_IFLAG = 1 << 2,
};

#define PWDFILE "/etc/passwd"
#define GRPFILE "/etc/group"

#define CSTRCMP(a, b) c_mem_cmp((a), sizeof((a)), (b))
#define ID(a, b) (((ctype_id)(a) == -1) ? (ctype_id)(b) : (ctype_id)(a))

struct install {
	ctype_id gid;
	ctype_id uid;
	uint mode;
	uint opts;
	uint ropts;
};

/* pwd: name:pass:uid:gid:dir:shell
 * grp: name:pass:gid:mem */
struct pdb {
	char *p[7];
};

ctype_status copy(char **, char *, uint, uint);
void deepsleep(u64);
ctype_status dbgetln(struct pdb *, ctype_ioq *, ctype_arr *);
size edyncat(ctype_arr *, void *, usize, usize);
size edynfmt(ctype_arr *, char *, ...);
vlong estrtovl(char *, int, vlong, vlong);
uvlong estrtouvl(char *, int, uvlong, uvlong);
ctype_id gidfromname(char *);
ctype_id gidfromuid(ctype_id);
ctype_status install(struct install *, char **, char *);
char *namefromgid(ctype_id);
char *namefromuid(ctype_id);
char *pathcat(char *, char *, int);
int prompt(char *);
ctype_status remove(char **, uint);
char **tmpargv(char *);
ctype_id uidfromname(char *);
int yesno(char *, char *);
