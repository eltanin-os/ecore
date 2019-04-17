typedef struct {
	CDent *dp;
	CNode *hp;
	int    depth;
	int    maxdepth;
} Dir;

int   dir_open(Dir *, CDir *, char *, uint);
void  dir_close(Dir *, CDir *);
vlong estrtovl(char *, int, vlong, vlong);
