/*
 * arch/mmix/kernel/irq.c
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

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/seq_file.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <asm/reg.h>

unsigned long savedK = 0;

static void dt_enable_irq(unsigned int irq)
{
	unsigned long mask = 1L << irq;

	savedK |= mask;
	if (!irqs_disabled())
		putspr(rK, savedK);
}

static void dt_disable_irq(unsigned int irq)
{
	unsigned long mask = 1L << irq;

	savedK &= ~mask;
	if (!irqs_disabled())
		putspr(rK, savedK);
}

static void dt_ack_irq(unsigned int irq)
{
	unsigned long Q;
	unsigned long mask = 1L << irq;

	savedK &= ~mask;
	Q = getspr(rQ);
	Q &= ~mask;
	putspr(rQ, Q);
}

static void dt_end_irq(unsigned int irq)
{
	unsigned long mask = 1L << irq;

	savedK |= mask;
}

struct hw_interrupt_type dt_interrupt = {
	.name		= "dynamic trap",
	.enable		= dt_enable_irq,
	.disable	= dt_disable_irq,
	.ack		= dt_ack_irq,
	.end		= dt_end_irq,
};

static int get_irq(void)
{
	unsigned long Q;
	unsigned long mask;
	int	i;

	Q = getspr(rQ);
	Q &= savedK;

	for (i = 32, mask = 1L << i; i < 40; i++, mask <<= 1)
		if (Q & mask)
			return i;

	for (i = 0, mask = 1; ; i++, mask <<= 1)
		if (Q & mask)
			return i;
}

void __init init_IRQ(void)
{
	int	i;

	for (i = 0; i < 64; i++) {
		set_irq_chip(i, &dt_interrupt);
	}
}

int show_interrupts(struct seq_file *p, void *v)
{
	extern void progress(const char * fmt, ...);
	progress("show_interrupt(): Not Yet\n");
	return 0;
}

static int is_interrupt(int irq)
{
	if ((irq >= 32) && (irq < 40))
		return 0;
	return 1;
}

void do_IRQ(void)
{
	struct	pt_regs	regs;
	int	irq = get_irq();

	regs.orig_rBB = 0;
	regs.rBB = getspr(rBB);
	regs.rWW = getspr(rWW);
	regs.rXX = getspr(rXX);
	regs.rYY = getspr(rYY);
	regs.rZZ = getspr(rZZ);

	if (is_interrupt(irq)) {
		irq_enter();
		generic_handle_irq(irq, &regs);
		irq_exit();
	} else {
		struct irqaction *action = irq_desc[irq].action;
		unsigned long Q;
		unsigned long mask = 1L << irq;

//printk("do_IRQ(): fault start. preempt_count = %d\n", preempt_count());
		Q = getspr(rQ);
		Q &= ~mask;
		putspr(rQ, Q);

		putspr(rK, savedK);
		action->handler(irq, action->dev_id, &regs);
		putspr(rK, 0);
//printk("do_IRQ(): fault done. preempt_count = %d\n", preempt_count());
	}
	if (!(regs.rWW & 0x8000000000000000)) {
		extern void do_work(struct pt_regs *regs);

		putspr(rK, savedK);
		do_work(&regs);
		putspr(rK, 0);
	}

	putspr(rBB, regs.rBB);
	putspr(rWW, regs.rWW);
	putspr(rXX, regs.rXX);
	putspr(rYY, regs.rYY);
	putspr(rZZ, regs.rZZ);
}
