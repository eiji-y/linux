/*
 * include/asm-mmix/processor.h
 *
 *   Copyright (C) 2008,2011 Eiji Yoshiya (eiji-y@pb3.so-net.ne.jp)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#ifndef _ASM_MMIX_PROCESSOR_H
#define _ASM_MMIX_PROCESSOR_H

#ifdef __KERNEL__

struct __aux_reg {
	unsigned long ksp;
	unsigned long ucp;	// user context pointer.
};

extern struct __aux_reg aux_reg;

struct task_struct;
void release_thread(struct task_struct *);
extern void prepare_to_copy(struct task_struct *tsk);

#define TASK_SIZE (0x8000000000000000UL)

typedef struct {
	unsigned long seg;
} mm_segment_t;

#define current_text_addr() ({ \
	extern void progress(const char * fmt, ...); \
	progress("current_text_addr(): Not Yet\n"); \
	(void *)0; \
})

#define cpu_relax()	barrier()

struct pt_regs;
extern void start_thread(struct pt_regs *regs, unsigned long start, unsigned long sp);
extern long kernel_thread(int (*fn)(void *), void *arg, unsigned long flags);

struct thread_struct {
	unsigned long	ksp;		/* Kernel stack pointer */
	mm_segment_t	fs;		/* for get_fs() validation */
	unsigned long	gpr[256];
	unsigned long	lr[256];	/* local registers */
};

#define INIT_THREAD  {							\
	.fs = KERNEL_DS, \
}

#define thread_saved_pc(tsk)    \
({ \
	extern void progress(const char * fmt, ...); \
	progress("thread_saved_pc(): Not Yet\n"); \
	(long)0; \
})

#define KSTK_EIP(task) \
({ \
	extern void progress(const char * fmt, ...); \
	progress("KSTK_EIP(): Not Yet\n"); \
	(long)0; \
})

#define KSTK_ESP(task) \
({ \
	extern void progress(const char * fmt, ...); \
	progress("KSTK_ESP(): Not Yet\n"); \
	(long)0; \
})

unsigned long get_wchan(struct task_struct *p);

#define TASK_UNMAPPED_BASE	(0x4000000000000000)

#endif /* __KERNEL__ */
#endif /* _ASM_MMIX_PROCESSOR_H */
