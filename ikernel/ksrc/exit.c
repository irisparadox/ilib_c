#include <kinclude/wait.h>
#include <kinclude/exit.h>
#include <kinclude/syscall.h>

SYSCALL_DEFINE2(wait4, tid_t, ktid, int *, wstatus)
{
	return 0;
}
