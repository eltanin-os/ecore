/* cp.c */
enum {
	CP_FFLAG = 1 << 0,
	CP_PFLAG = 1 << 1,
	CP_RFLAG = 1 << 2,
	CP_TDIR  = 1 << 3,
};

ctype_status copy(char **, char *, uint, uint);

/* estrtovl.c */
vlong estrtovl(char *, int, vlong, vlong);

/* path.c */
void trim_trailing_slash(char *);

/* pathcat.c */
char *pathcat(char *, char *, int);

/* rm.c */
enum {
	RM_FFLAG = 1 << 0,
	RM_RFLAG = 1 << 1,
};

ctype_status remove(char **, uint);

/* sdup.c */
char *sdup(char *);

/* serr.c */
char *serr(int);

/* strtomode.c */
uint strtomode(char *, uint, uint);

/* tmpargv.c */
char **tmpargv(char *);
