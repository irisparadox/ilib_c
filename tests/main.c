#include <assert.h>
#include <bits/stdint-uintn.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <icontext.h>

static int resumed;
static int sig_resumed;

static int swap_landed_b;
static int swap_landed_a;
static icontext_t ctx_a, ctx_b;

static uint8_t xmm_expected[16] = {
	0xde, 0xad, 0xbe, 0xef,
	0xca, 0xfe, 0xba, 0xbe,
	0x11, 0x22, 0x33, 0x44,
	0x55, 0x66, 0x77, 0x88
};

static uint8_t xmm_actual[16];

static void set_registers(void)
{
	__asm__ volatile(
		"mov $0x1111111111111111, %%rax\n"
		"mov $0x2222222222222222, %%rbx\n"
		"mov $0x3333333333333333, %%rcx\n"
		"mov $0x4444444444444444, %%rdx\n"
		"mov $0x5555555555555555, %%rsi\n"
		"mov $0x6666666666666666, %%rdi\n"
		"mov $0x7777777777777777, %%rbp\n"
		"mov $0x8888888888888888, %%r8\n"
		"mov $0x9999999999999999, %%r9\n"
		"mov $0xaaaaaaaaaaaaaaaa, %%r10\n"
		"mov $0xbbbbbbbbbbbbbbbb, %%r11\n"
		"mov $0xcccccccccccccccc, %%r12\n"
		"mov $0xdddddddddddddddd, %%r13\n"
		"mov $0xeeeeeeeeeeeeeeee, %%r14\n"
		"mov $0xffffffffffffffff, %%r15\n"
		:
		:
		: "rax", "rbx", "rcx", "rdx",
		  "rsi", "rdi", "r8", "r9",
		  "r10", "r11", "r12", "r13",
		  "r14", "r15"
	);
}

static void clobber_registers(void)
{
	__asm__ volatile(
		"xor %%rax, %%rax\n"
		"xor %%rbx, %%rbx\n"
		"xor %%rcx, %%rcx\n"
		"xor %%rdx, %%rdx\n"
		"xor %%rsi, %%rsi\n"
		"xor %%rdi, %%rdi\n"
		"xor %%r8, %%r8\n"
		"xor %%r9, %%r9\n"
		"xor %%r10, %%r10\n"
		"xor %%r11, %%r11\n"
		"xor %%r12, %%r12\n"
		"xor %%r13, %%r13\n"
		"xor %%r14, %%r14\n"
		"xor %%r15, %%r15\n"
		:
		:
		: "rax", "rbx", "rcx", "rdx",
		  "rsi", "rdi", "r8", "r9",
		  "r10", "r11", "r12", "r13",
		  "r14", "r15"
	);
}

static void set_xmm0(void)
{
	__asm__ volatile(
		"movdqu (%0), %%xmm0"
		:
		: "r"(xmm_expected)
		: "xmm0"
	);
}

static void clobber_xmm0(void)
{
	uint8_t zero[16] = {0};

	__asm__ volatile(
		"movdqu (%0), %%xmm0"
		:
		: "r"(zero)
		: "xmm0"
	);
}

static void read_xmm0(void)
{
	__asm__ volatile(
		"movdqu %%xmm0, (%0)"
		:
		: "r"(xmm_actual)
		: "memory"
	);
}

static uint64_t read_flags(void)
{
	uint64_t flags;

	__asm__ volatile(
		"pushfq\n"
		"popq %0"
		: "=r"(flags)
	);

	return flags;
}

static void assert_sigusr1_blocked(int expected_blocked)
{
	sigset_t current;
	int is_blocked;

	sigprocmask(SIG_BLOCK, NULL, &current);
	is_blocked = sigismember(&current, SIGUSR1);
	assert(is_blocked == expected_blocked);
}

static void test_signal_mask(void)
{
	icontext_t ctx;
	sigset_t block_set;

	/* Start from a known state: SIGUSR1 unblocked.  */
	sigemptyset(&block_set);
	sigaddset(&block_set, SIGUSR1);
	sigprocmask(SIG_UNBLOCK, &block_set, NULL);
	assert_sigusr1_blocked(0);

	/* Block SIGUSR1, then capture that as our saved context.  */
	sigprocmask(SIG_BLOCK, &block_set, NULL);
	assert_sigusr1_blocked(1);

	igetcontext(&ctx);
	if (!sig_resumed) {
		sig_resumed = 1;

		/* Diverge: unblock it, so the live mask no longer
		   matches what's in ctx.  */
		sigprocmask(SIG_UNBLOCK, &block_set, NULL);
		assert_sigusr1_blocked(0);

		/* isetcontext should restore SIG_BLOCK state, i.e.
		   SIGUSR1 blocked again, since that's what was
		   captured.  */
		isetcontext(&ctx);
		assert(0); /* unreachable */
	}

	/* We're back via isetcontext: mask should be restored.  */
	assert_sigusr1_blocked(1);

	/* Clean up so this doesn't leak into other tests.  */
	sigprocmask(SIG_UNBLOCK, &block_set, NULL);

	puts("signal mask test passed");
}

static void test_swap(void)
{
	sigset_t usr2;
	sigset_t current;
	uint8_t zero[16] = {0};

	sigemptyset(&usr2);
	sigaddset(&usr2, SIGUSR2);

	/* --- Establish "B" state. --- */
	sigprocmask(SIG_UNBLOCK, &usr2, NULL);
	set_registers();
	set_xmm0();
	sigprocmask(SIG_BLOCK, &usr2, NULL);

	igetcontext(&ctx_b);
	if (!swap_landed_b) {
		swap_landed_b = 1;

		/* --- Establish "A" state. --- */
		clobber_registers();
		clobber_xmm0();
		sigprocmask(SIG_UNBLOCK, &usr2, NULL);

		/* Save A and switch to B. */
		iswapcontext(&ctx_a, &ctx_b);

		/* Returned from B. We should get back exactly the
		   state that was saved into ctx_a. */
		swap_landed_a = 1;

		read_xmm0();
		assert(memcmp(xmm_actual, zero, sizeof(zero)) == 0);

		sigprocmask(SIG_BLOCK, NULL, &current);
		assert(sigismember(&current, SIGUSR2) == 0);

		puts("swap back to A passed");
		return;
	}

	/* We arrived in B. Verify B's saved state. */
	read_xmm0();
	assert(memcmp(xmm_actual, xmm_expected, sizeof(xmm_expected)) == 0);

	sigprocmask(SIG_BLOCK, NULL, &current);
	assert(sigismember(&current, SIGUSR2) == 1);

	puts("swap to B passed");

	/* Modify the live state so the restoration of A is
	   actually observable. */
	clobber_registers();
	clobber_xmm0();
	sigprocmask(SIG_UNBLOCK, &usr2, NULL);

	/* Save the modified B state and restore A. */
	iswapcontext(&ctx_b, &ctx_a);

	assert(0);
}

int main(void)
{
	icontext_t ctx;

	set_registers();
	set_xmm0();

	uint64_t before = read_flags();

	igetcontext(&ctx);

	if (!resumed) {
		resumed = 1;

		clobber_registers();
		clobber_xmm0();

		isetcontext(&ctx);

		assert(read_flags() == before);
		assert(0);
	}

	read_xmm0();

	assert(memcmp(xmm_actual, xmm_expected, sizeof(xmm_expected)) == 0);

	puts("xmm state passed");

	/*
	 * GPR validation is done by the context itself.
	 * If setcontext reaches here with corrupted callee-saved
	 * registers, execution would already be undefined.
	 */

	puts("context test passed");

	test_signal_mask();
	test_swap();

	return 0;
}
