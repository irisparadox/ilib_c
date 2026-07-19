#include <deftypei.h>
#include <signal.h>
#include <stdio.h>
#include <icontext.h>

#define icontext(member)	ILIB_OFFSETOF (icontext_t, member)
#define imcontext(member)	icontext      (ic_mcontext.member)
#define imreg(reg)		imcontext     (gregs[IMREG_##reg])

int main(void)
{
	puts("/* Automatically generated. Do not edit. */");
	puts("#ifndef I_CONTEXT_ASM_H_");
	puts("#define I_CONTEXT_ASM_H_");

	puts("");

	printf("#define SIG_BLOCK %d\n", SIG_BLOCK);
	printf("#define SIG_SETMASK %d\n", SIG_SETMASK);
	printf("#define IC_NSIG8 %d\n", _NSIG / 8);

	puts("");

	printf("#define oiR8      %zu\n", imreg(R8));
	printf("#define oiR9      %zu\n", imreg(R9));
	printf("#define oiR12     %zu\n", imreg(R12));
	printf("#define oiR13     %zu\n", imreg(R13));
	printf("#define oiR14     %zu\n", imreg(R14));
	printf("#define oiR15     %zu\n", imreg(R15));
	printf("#define oiRDI     %zu\n", imreg(RDI));
	printf("#define oiRSI     %zu\n", imreg(RSI));
	printf("#define oiRBP     %zu\n", imreg(RBP));
	printf("#define oiRBX     %zu\n", imreg(RBX));
	printf("#define oiRDX     %zu\n", imreg(RDX));
	printf("#define oiRCX     %zu\n", imreg(RCX));
	printf("#define oiRSP     %zu\n", imreg(RSP));
	printf("#define oiRIP     %zu\n", imreg(RIP));
	printf("#define oiRFLAGS  %zu\n", imreg(RFLAGS));

	puts("");

	printf("#define oiFPREGS %zu\n", imcontext(fpregs));
	printf("#define oiSIGMASK %zu\n", icontext(ic_sigmask));

	puts("");

	puts("#endif");
}
