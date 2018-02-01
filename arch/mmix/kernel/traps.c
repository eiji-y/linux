/*
 * arch/mmix/kernel/traps.c
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
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/irq.h>

#include <asm/reg.h>
#include <asm/uaccess.h>

typedef	long syscall_handler_t(unsigned long arg1, unsigned long arg2,
			       unsigned long arg3, unsigned long arg4,
			       unsigned long arg5, unsigned long arg6,
				struct pt_regs *regs);

extern syscall_handler_t *sys_call_table[];
extern long nr_syscalls;

void __init trap_init(void)
{
	extern	void forced_trap(void);
	extern	void dynamic_trap(void);

//	printk("trap_init(): Not Yet\n");

	putspr(rT,forced_trap);
	putspr(rTT,dynamic_trap);
}

void do_work(struct pt_regs *regs)
{
recheck:
	if (test_thread_flag(TIF_NEED_RESCHED)) {
		// printk("need rescedule\n");
		schedule();
	}
	if (test_thread_flag(TIF_SIGPENDING)) {
//		printk("singal pending.\n");
		do_signal(regs);
		goto recheck;
	}
}

void do_syscall(void)
{
	struct	pt_regs	regs;
	unsigned long num;
	unsigned long xx;
	unsigned long __user *arg;
	unsigned long args[6];

	regs.orig_rBB = getspr(rBB);
	regs.rWW = getspr(rWW);
	regs.rXX = getspr(rXX);
	regs.rYY = getspr(rYY);
	regs.rZZ = getspr(rZZ);
	putspr(rK, savedK);

	arg = (unsigned long __user *)regs.orig_rBB;
	regs.rBB = -ENOSYS;

//	printk("do_syscall(): nr_syscalls = %ld\n", nr_syscalls);
//	printk("rYY = %ld, rZZ = %ld\n", regs.rYY, regs.rZZ);

	xx = (regs.rXX >> 16) & 0xff;

	if (xx != 1) {
		printk("invalid trap %ld,%ld,%ld at %lx\n",
			xx, regs.rYY, regs.rZZ, regs.rWW);
	} else {
		num = regs.rYY * 256 + regs.rZZ;
		if (num == 65535) {
//			num = *args++;
			if (get_user(num, arg++)) {
				regs.rBB = -EFAULT;
				goto out;
			}
		}
//	printk("syscall num = %ld\n", num);
//	printk("arg0 = %lx\n", args[0]);
//	printk("arg1 = %lx\n", args[1]);
//	printk("arg2 = %lx\n", args[2]);
		if (num < nr_syscalls) {
//			printk("syscall %p, %ld, %lx\n", current, num, regs.rWW);
			if (copy_from_user(args, arg, sizeof(args))) {
				regs.rBB = -EFAULT;
				goto out;
			}
			regs.rBB = (*sys_call_table[num])(
				args[0], args[1], args[2],
				args[3], args[4], args[5],
				&regs);
//			printk("syscall %p, %ld, %lx = %lx\n", current, num, regs.rWW, regs.rBB);
		} else {
			printk("invalid trap %ld,%ld,%ld at %lx\n",
				xx, regs.rYY, regs.rZZ, regs.rWW);
		}
	}

out:
//	printk("do_syscall(): ret = %ld\n", regs.rBB);
	do_work(&regs);

	putspr(rK, 0);
	putspr(rBB, regs.rBB);
	putspr(rWW, regs.rWW);
	putspr(rXX, regs.rXX);
	putspr(rYY, regs.rYY);
	putspr(rZZ, regs.rZZ);
}

void show_stack(struct task_struct *task, unsigned long *esp)
{
	extern void progress(const char * fmt, ...);
	progress("show_stack(): Not Yet\n");
}
