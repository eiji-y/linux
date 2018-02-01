/*
 * include/asm-mmix/mmu_context.h
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

#ifndef __ASM_MMIX_MMU_CONTEXT_H
#define __ASM_MMIX_MMU_CONTEXT_H

#include <linux/kernel.h>
#include <asm/reg.h>

#ifdef __KERNEL__

static inline void enter_lazy_tlb(struct mm_struct *mm,
				  struct task_struct *tsk)
{
}

static inline int init_new_context(struct task_struct *t, struct mm_struct *mm)
{
//	printk("init_new_context(t = %p, mm = %p): Not Yet\n", t, mm);
	return 0;
}

static inline void destroy_context(struct mm_struct *mm)
{
//	printk("destroy_context(mm = %p): Not Yet\n", mm);
}

static inline void switch_mm(struct mm_struct *prev, struct mm_struct *next,
			     struct task_struct *tsk)
{
//	printk("switch_mm(prev = %p, next = %p): Not Yet\n", prev, next);

	if (prev == next)
		return;

//	printk("switch_mm(): next pgd = %p, __pa(pgd) = %lx\n", next->pgd, __pa(next->pgd));
	// load rV
	putspr(rV, __pa(next->pgd)|0x24680d0000000000);
	// flush tlb
	asm("	sync	6");
}

#define deactivate_mm(tsk,mm)	do { } while (0)

static inline void activate_mm(struct mm_struct *prev, struct mm_struct *next)
{
	switch_mm(prev, next, NULL);
}

#endif /* __KERNEL__ */
#endif /* __ASM_MMIX_MMU_CONTEXT_H */
