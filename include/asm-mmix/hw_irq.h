/*
 * include/asm-mmix/hw_irq.h
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

#ifndef _ASM_MMIX_HW_IRQ_H
#define _ASM_MMIX_HW_IRQ_H

#ifdef __KERNEL__

#include <linux/irqreturn.h>
#include <asm/ptrace.h>


extern unsigned long savedK;
extern unsigned long intrEn;

extern void mymessage(char *);

static inline void local_irq_disable(void)
{
	unsigned long rK = 0;

	__asm__ __volatile__("put rK,%0"::"r"(rK):"memory");
	intrEn = 0;
}

static inline void local_irq_enable(void)
{
	unsigned long rK = savedK;

	intrEn = 1;
	__asm__ __volatile__("put rK,%0"::"r"(rK):"memory");
}

static inline void local_irq_save_ptr(unsigned long *flags)
{
	*flags = intrEn;
	local_irq_disable();
}

#define local_irq_save(flags)	local_irq_save_ptr(&flags)

static inline void local_irq_restore(unsigned long flags)
{
	unsigned long rK = (intrEn = flags)?savedK:0;

	__asm__ __volatile__("put rK,%0"::"r"(rK):"memory");
}

static inline unsigned long local_get_flags(void)
{
	return intrEn;
}

#define local_save_flags(flags)	((flags) = local_get_flags())
#define irqs_disabled()		(local_get_flags() == 0)

#endif	/* __KERNEL__ */
#endif	/* _ASM_MMIX_HW_IRQ_H */
