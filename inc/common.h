enum {
	/* cp flags */
	CP_FFLAG = 1 << 0,
	CP_PFLAG = 1 << 1,
	CP_RFLAG = 1 << 2,
	CP_TDIR  = 1 << 3,

	/* rm flags */
	RM_FFLAG = 1 << 0,
	RM_RFLAG = 1 << 1,
};

#define GBUFSIZ C_PATHMAX
#define sdup(a) sndup((a), c_str_len((a), GBUFSIZ))

ctype_status copy(char **, char *, uint, uint);
vlong estrtovl(char *, int, vlong, vlong);
char *pathcat(char *, char *, int);
ctype_status remove(char **, uint);
char *sndup(char *, usize);
char *serr(int);
uint strtomode(char *, uint, uint);
char **tmpargv(char *);
void trim_trailing_slash(char *);

extern char *gbuf;
