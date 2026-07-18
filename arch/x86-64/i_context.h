#ifndef I_CONTEXT_H_
#define I_CONTEXT_H_

#include <deftypei.h>

#define IMREG_R8      0
#define IMREG_R9      1
#define IMREG_R12     2
#define IMREG_R13     3
#define IMREG_R14     4
#define IMREG_R15     5
#define IMREG_RDI     6
#define IMREG_RSI     7
#define IMREG_RBP     8
#define IMREG_RBX     9
#define IMREG_RDX     10
#define IMREG_RCX     11
#define IMREG_RSP     12
#define IMREG_RIP     13
#define IMREG_RFLAGS  14
#define IMREG_NGREG   15

typedef ilib_uintptr_t imreg_t;
typedef imreg_t        imregset_t[IMREG_NGREG];

typedef struct imcontext_t {
	imregset_t   gregs;
	ilib_uint8_t fpregs[512] ILIB_ALIGN(16);
} imcontext_t;

#endif // I_CONTEXT_H_
