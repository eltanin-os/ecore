#include <tertium/cpu.h>
#include <tertium/std.h>

void
head(ctype_fd fd, char *fname, usize cnt)
{
	ctype_ioq ioq;
	size r;
	char buf[C_BIOSIZ];

	r = 1;
	c_ioq_init(&ioq, fd, buf, sizeof(buf), c_sys_read);
	for (; r && cnt;){
		if ((r = c_ioq_getln(&ioq, c_ioq_arr(ioq1))) < 0) {
			if (errno != C_ENOMEM)
				c_err_die(1, "c_ioq_getln %s", fname);
			c_ioq_flush(ioq1);
			continue;
		}
		--cnt;
	}
}
