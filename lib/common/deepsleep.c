#include <tertium/cpu.h>
#include <tertium/std.h>

#include "common.h"

void
deepsleep(u64int time)
{
	ctype_taia deadline, stamp;

	c_taia_u64(&deadline, time);
	c_taia_now(&stamp);
	c_taia_add(&deadline, &stamp, &deadline);
	c_nix_deepsleep(&deadline, &stamp);
}
