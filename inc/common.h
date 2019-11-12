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

ctype_status copy(char **, char *, uint, uint);
vlong estrtovl(char *, int, vlong, vlong);
char *pathcat(char *, char *, int);
ctype_status remove(char **, uint);
char *sdup(char *);
char *serr(int);
uint strtomode(char *, uint, uint);
char **tmpargv(char *);
void trim_trailing_slash(char *);

extern char *gbuf;
