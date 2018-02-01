/*
 * include/asm-mmix/system.h
 *
 *   Copyright (C) 2008 Eiji Yoshiya (eiji-y@pb3.so-net.ne.jp)
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

#ifndef _ASM_MMIX_SYSTEM_H
#define _ASM_MMIX_SYSTEM_H

#ifdef __KERNEL__

#include <linux/kernel.h>

#define mb()  __asm__ __volatile__ ("sync 3" : : : "memory")

#define wmb()  do { \
	extern void progress(const char * fmt, ...); \
	progress("wmb(): Not Yet\n"); \
} while (0)

#define set_mb(var, value)	do { var = value; mb(); } while (0)

#define smp_mb()	barrier()
#define smp_rmb()	barrier()
#define smp_wmb()	barrier()
#define smp_read_barrier_depends()	do { } while(0)

#define xchg(ptr,x) ((__typeof__(*(ptr)))__xchg((unsigned long)(x),(ptr),sizeof(*(ptr))))

static inline unsigned long __xchg(unsigned long x, volatile void *ptr, int size)
{
	unsigned long val;

	__asm__ __volatile__("\
1:	ldo	%0,%1,0\n\
	put	rP,%0\n\
	set	%0,%2\n\
	cswap	%0,%1,0\n\
	bz	%0,1b\n\
	get	%0,rP"
	:"=&r"(val)
	:"r"(ptr),"r"(x)
	:"memory");

	switch (size) {
	case 1:
		val >>= (7 - ((unsigned long)ptr & 7)) * 8;
		val &= 0xff;
		break;
	case 2:
		val >>= (6 - ((unsigned long)ptr & 6)) * 8;
		val &= 0xffff;
		break;
	case 4:
		val >>= (4 - ((unsigned long)ptr & 4)) * 8;
		val &= 0xffffffff;
		break;
	}
	return val;
}

extern struct task_struct *__switch_to(struct task_struct *,
	struct task_struct *);
#define switch_to(prev, next, last)	((last) = __switch_to((prev), (next)))

struct thread_struct;
extern struct task_struct *_switch(struct thread_struct *prev,
				   struct thread_struct *next);

#define arch_align_stack(x) (x)

#endif /* __KERNEL__ */
#endif	/* _ASM_MMIX_SYSTEM_H */
