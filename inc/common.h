typedef struct {
	CDent *dp;
	CNode *hp;
	int    depth;
	int    maxdepth;
} Dir;

#define pathcat(a, b)  pathcatf(nil, 0, c_gen_basename((a)), (b), -1)
#define pathcatx(a, b) pathcatf(nil, 0, c_gen_basename((a)), (b),  1)

int    dir_open(Dir *, CDir *, char *, uint);
void   dir_close(Dir *, CDir *);
vlong  estrtovl(char *, int, vlong, vlong);
char * pathcatf(char *, usize, char *, char *, int);
