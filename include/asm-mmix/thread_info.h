/* thread_info.h: i386 low-level thread information
 *
 * Copyright (C) 2002  David Howells (dhowells@redhat.com)
 * - Incorporating suggestions made by Linus Torvalds and Dave Miller
 */

#ifndef _ASM_MMIX_THREAD_INFO_H
#define _ASM_MMIX_THREAD_INFO_H

#define THREAD_SIZE		(8192 * 2)

#ifndef __ASSEMBLY__

struct thread_info {
	struct task_struct	*task;		/* main task structure */
	struct exec_domain *exec_domain;	/* execution domain */
	int		preempt_count;		/* 0 => preemptable, <0 => BUG */
	struct restart_block restart_block;
	unsigned long		flags;		/* low level flags */
};

#define INIT_THREAD_INFO(tsk)			\
{						\
	.task =		&tsk,			\
	.exec_domain =	&default_exec_domain,	\
	.preempt_count = 1,			\
	.restart_block = {			\
		.fn = do_no_restart_syscall,	\
	},					\
	.flags =	0,			\
}

#define init_thread_info	(init_thread_union.thread_info)

#define alloc_thread_info(tsk)	kmalloc(THREAD_SIZE, GFP_KERNEL)
#define free_thread_info(ti)	kfree(ti)

#endif /* __ASSEMBLY__ */

#ifndef __ASSEMBLY__
static inline struct thread_info *current_thread_info(void)
{
	register unsigned long sp asm("$254");

	return (struct thread_info *)(sp & ~(THREAD_SIZE-1));
}
#endif /* __ASSEMBLY__ */

#define PREEMPT_ACTIVE		0x10000000

#define TIF_SYSCALL_TRACE	0	/* syscall trace active */
#define TIF_NOTIFY_RESUME	1	/* resumption notification requested */
#define TIF_SIGPENDING		2	/* signal pending */
#define TIF_NEED_RESCHED	3	/* rescheduling necessary */

#define TIF_MEMDIE		10
#define TIF_RESTORE_SIGMASK	15	/* Restore signal mask in do_signal */

#endif	/* _ASM_MMIX_THREAD_INFO_H */
