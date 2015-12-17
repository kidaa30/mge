
#include <base/printf.h>

extern void create_libc_comm_plugin();

void __attribute__((constructor)) init_libc_comm(void)
{
	PDBG("init_libc_comm()\n");
	create_libc_comm_plugin();
}
