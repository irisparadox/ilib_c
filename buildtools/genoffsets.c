#include <stdio.h>
#include <icontext.h>

int main(void)
{
	puts("/* Automatically generated. Do not edit. */");
	puts("#ifndef I_CONTEXT_ASM_H_");
	puts("#define I_CONTEXT_ASM_H_");
	puts("");

	printf("#define IMREG_R8      %d\n", IMREG_R8);
	printf("#define IMREG_R9      %d\n", IMREG_R9);
	printf("#define IMREG_R10     %d\n", IMREG_R10);
	printf("#define IMREG_R11     %d\n", IMREG_R11);
	printf("#define IMREG_R12     %d\n", IMREG_R12);
	printf("#define IMREG_R13     %d\n", IMREG_R13);
	printf("#define IMREG_R14     %d\n", IMREG_R14);
	printf("#define IMREG_R15     %d\n", IMREG_R15);
	printf("#define IMREG_RDI     %d\n", IMREG_RDI);
	printf("#define IMREG_RSI     %d\n", IMREG_RSI);
	printf("#define IMREG_RBP     %d\n", IMREG_RBP);
	printf("#define IMREG_RBX     %d\n", IMREG_RBX);
	printf("#define IMREG_RDX     %d\n", IMREG_RDX);
	printf("#define IMREG_RAX     %d\n", IMREG_RAX);
	printf("#define IMREG_RCX     %d\n", IMREG_RCX);
	printf("#define IMREG_RSP     %d\n", IMREG_RSP);
	printf("#define IMREG_RIP     %d\n", IMREG_RIP);
	printf("#define IMREG_RFLAGS  %d\n", IMREG_RFLAGS);
	printf("#define IMREG_COUNT   %d\n", IMREG_NGREG);

	puts("");

	printf("#define IC_GREGS %zu\n",
		ILIB_OFFSETOF(icontext_t, ic_mcontext) +
		ILIB_OFFSETOF(imcontext_t, gregs));

	printf("#define IC_FPREGS %zu\n",
		ILIB_OFFSETOF(icontext_t, ic_mcontext) +
		ILIB_OFFSETOF(imcontext_t, fpregs));

	puts("");
	puts("#endif");
}
