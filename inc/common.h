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

#define edyncat(a, b, c, d) \
if (c_dyn_cat((a), (b), (c), (d)) < 0) c_err_die(1, "c_dyn_cat");

#define edynfmt(a, b, ...) \
if (c_dyn_fmt((a), (b), __VA_ARGS__) < 0) c_err_die(1, "c_dyn_fmt");

#define CSTRCMP(a, b) c_mem_cmp((a), sizeof((a)), (b))
#define ID(a, b) (((a) == (uint)-1) ? (b) : (a))

struct install {
	ctype_id gid;
	ctype_id uid;
	uint mode;
	uint opts;
	uint ropts;
};

ctype_status copy(char **, char *, uint, uint);
vlong estrtovl(char *, int, vlong, vlong);
ctype_id gidfromname(char *);
ctype_status install(struct install *, char **, char *);
ctype_status mkpath(char *, uint, uint);
char *namefromgid(ctype_id);
char *namefromuid(ctype_id);
char *pathcat(char *, char *, int);
int prompt(char *);
ctype_status remove(char **, uint);
char *serr(int);
uint strtomode(char *, uint, uint);
char **tmpargv(char *);
void trim_trailing_slash(char *);
ctype_id uidfromname(char *);
int yesno(char *);
