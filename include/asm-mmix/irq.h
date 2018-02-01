/*
 * include/asm-mmix/irq.h
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

#ifndef _ASM_MMIX_IRQ_H
#define _ASM_MMIX_IRQ_H
#ifdef __KERNEL__

#include <linux/kernel.h>

#define NR_IRQS		64

static inline void ack_bad_irq(int irq)
{
        printk(KERN_CRIT "illegal vector %d received!\n", irq);
        BUG();
}

static __inline__ int irq_canonicalize(int irq)
{
	return irq;
}

#endif /* __KERNEL__ */
#endif /* _ASM_MMIX_IRQ_H */
