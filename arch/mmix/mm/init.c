/*
 * arch/mmix/mm/init.c
 *
 *   Copyright (C) 2008-2009 Eiji Yoshiya (eiji-y@pb3.so-net.ne.jp)
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

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/bootmem.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <asm/tlb.h>

DEFINE_PER_CPU(struct mmu_gather, mmu_gathers);

void free_initmem(void)
{
	printk("free_initmem(): Not Yet\n");
}

static irqreturn_t page_fault(int irq, void *data, struct pt_regs *regs)
{
	extern void do_page_fault(struct pt_regs *regs, int code);

	do_page_fault(regs, 1 << (irq - 37));
        return IRQ_HANDLED;
}

static struct irqaction irq_pgfault  = { page_fault, IRQF_DISABLED, CPU_MASK_NONE, "page fault", NULL, NULL};

void __init mem_init(void)
{
	extern unsigned long boot_mem_size;

	max_mapnr = boot_mem_size >> PAGE_SHIFT;

	totalram_pages += free_all_bootmem();
	printk("mem_init(): totalram_pages = %lx\n", totalram_pages);

	setup_irq(37, &irq_pgfault);
	setup_irq(38, &irq_pgfault);
	setup_irq(39, &irq_pgfault);
}
