#include <stdio.h>
#include <icontext.h>

int main(void)
{
	printf("#define IC_RBX %zu\n",
		ILIB_OFFSETOF(icontext_t, ic_mcontext) +
		ILIB_OFFSETOF(imcontext_t, rbx));

	printf("#define IC_RBP %zu\n",
		ILIB_OFFSETOF(icontext_t, ic_mcontext) +
		ILIB_OFFSETOF(imcontext_t, rbp));

	printf("#define IC_R12 %zu\n",
		ILIB_OFFSETOF(icontext_t, ic_mcontext) +
		ILIB_OFFSETOF(imcontext_t, r12));

	printf("#define IC_R13 %zu\n",
		ILIB_OFFSETOF(icontext_t, ic_mcontext) +
		ILIB_OFFSETOF(imcontext_t, r13));

	printf("#define IC_R14 %zu\n",
		ILIB_OFFSETOF(icontext_t, ic_mcontext) +
		ILIB_OFFSETOF(imcontext_t, r14));

	printf("#define IC_R15 %zu\n",
		ILIB_OFFSETOF(icontext_t, ic_mcontext) +
		ILIB_OFFSETOF(imcontext_t, r15));

	printf("#define IC_RSP %zu\n",
		ILIB_OFFSETOF(icontext_t, ic_mcontext) +
		ILIB_OFFSETOF(imcontext_t, rsp));

	printf("#define IC_RIP %zu\n",
		ILIB_OFFSETOF(icontext_t, ic_mcontext) +
		ILIB_OFFSETOF(imcontext_t, rip));

	return 0;
}
