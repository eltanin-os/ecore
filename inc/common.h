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

/* pathcat.c */
char *pathcat(char *, char *, int);

/* sdup.c */
char *sdup(char *);

/* serr.c */
char *serr(int);
