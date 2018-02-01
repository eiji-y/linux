/*
 * arch/mmix/kernel/time.c
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
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/time.h>
#include <linux/interrupt.h>
#include <linux/irq.h>

#include <asm/reg.h>

irqreturn_t timer_interrupt(int irq, void *data, struct pt_regs *regs)
{
	putspr(rI, 5000);
	update_process_times(user_mode(regs));
	do_timer(regs);
	return IRQ_HANDLED;
}

struct rtc {
	long	cmd;
	long	sec;		/* seconds */
	long	usec;		/* microseconds */
};

static  volatile struct rtc *__rtc = (struct rtc *)0x8001000000000010;

void do_gettimeofday(struct timeval *tv)
{
	// read rtc.
	
	__rtc->cmd = 1;
	while (__rtc->cmd)
		;
	tv->tv_sec = __rtc->sec;
	tv->tv_usec = __rtc->usec;
}

int do_settimeofday(struct timespec *tv)
{
	extern void progress(const char * fmt, ...);
	progress("do_settimeofday(): Not Yet\n");
	return 0;
}

unsigned long long sched_clock(void)
{
	long	cc;

	asm	("get	%0, rC":"=&r"(cc));
	return cc;
}

static struct irqaction irq7  = { timer_interrupt, IRQF_DISABLED, CPU_MASK_NONE, "timer", NULL, NULL};

void __init time_init(void)
{
	printk("time_init(): Not Yet\n");

	setup_irq(7, &irq7);
	putspr(rI, 5000);
}
