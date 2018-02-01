#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/thread_info.h>
#include <asm/processor.h>

#define DEFINE(sym, val) \
        asm volatile("\n->" #sym " %0 " #val : : "i" (val))

void foo(void)
{
	DEFINE(THREAD, offsetof(struct task_struct, thread));
	DEFINE(THREAD_INFO_SIZE, sizeof(struct thread_info));

	DEFINE(KSP, offsetof(struct thread_struct, ksp));
	DEFINE(AUX_REG_SIZE, sizeof(struct __aux_reg));
	DEFINE(AUX_REG_KSP, offsetof(struct __aux_reg, ksp));
	DEFINE(AUX_REG_UCP, offsetof(struct __aux_reg, ucp));
}
