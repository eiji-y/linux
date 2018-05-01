/*
 * arch/mmix/kernel/signal.c
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

#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/signal.h>
#include <linux/errno.h>
#include <asm/unistd.h>
#include <asm/uaccess.h>
#include <asm/processor.h>

#define	_BLOCKABLE (~(sigmask(SIGKILL) | sigmask(SIGSTOP)))

struct sigframe {
	// NOT YET temporary global registers.
	unsigned long r255;
	unsigned long rWW;
	unsigned long rBB;
	unsigned long oldmask;
	unsigned int retcode[1];
};

long sys_sigaction(int sig, struct old_sigaction __user *act,
		struct old_sigaction __user *oact)
{
	int ret;

	printk("sys_sigaction(): Not Yet\n");
	for (;;)
		;
	ret = -EFAULT;
	return ret;
}

long sys_sigsuspend(old_sigset_t mask)
{
	printk("sys_sigsuspend(): Not Yet\n");
	for (;;)
		;
 	return -ERESTARTNOHAND;
}

long sys_sigreturn(int r3, int r4, int r5, int r6, int r7, int r8,
		       struct pt_regs *regs)
{
	struct sigframe __user *frame;
	sigset_t set;
	unsigned int err = 0;

	frame = (struct sigframe __user*)current->thread.gpr[254];

	if (!access_ok(VERIFY_READ, frame, sizeof(*frame)))
		goto badframe;
	if (__get_user(set.sig[0], &frame->oldmask))
		goto badframe;

	sigdelsetmask(&set, ~_BLOCKABLE);
	spin_lock_irq(&current->sighand->siglock);
	current->blocked = set;
	recalc_sigpending();
	spin_unlock_irq(&current->sighand->siglock);

	current_thread_info()->restart_block.fn = do_no_restart_syscall;

	err |= __get_user(regs->rBB, &frame->rBB);
	err |= __get_user(regs->rWW, &frame->rWW);
	err |= __get_user(current->thread.gpr[255], &frame->r255);

	current->thread.gpr[254] += (sizeof(*frame) + 7) & ~7;

 	return 0;

badframe:
printk("sys_sigreturn: badframe\n");
	force_sig(SIGSEGV, current);
	return 0;
}

long sys_rt_sigreturn(int r3, int r4, int r5, int r6, int r7, int r8,
		     struct pt_regs *regs)
{
	printk("sys_rt_sigreturn(): Not Yet\n");
	for (;;)
		;
 	return 0;
}

long sys_sigaltstack(const stack_t __user *uss, stack_t __user *uoss, int r5,
		int r6, int r7, int r8, struct pt_regs *regs)
{
	printk("sys_sigaltstack(): Not Yet\n");
	for (;;)
		;
 	return 0;
}

static inline void __user *get_sigframe(struct k_sigaction *ka,
			struct pt_regs *regs, size_t frame_size)
{
	unsigned long newsp;

	newsp = current->thread.gpr[254];

	if (ka->sa.sa_flags & SA_ONSTACK) {
		if (sas_ss_flags(newsp) == 0)
			newsp = current->sas_ss_sp + current->sas_ss_size;
	}

	newsp -= frame_size;
	newsp &= -8ul;

	return (void __user *)newsp;
}

static int flush_registerstack(int newL)
{
	int i;
	unsigned long __user *rS, *rO;
	unsigned long rL;

	rO = (unsigned long __user *)current->thread.gpr[10];
	rS = (unsigned long __user *)current->thread.gpr[11];
	rL = current->thread.gpr[20];

	rO += rL;

	for (i = 0; rS != rO; i++, rS++) {
		if (__put_user(current->thread.lr[i], rS))
			return -EFAULT;
	}
	if (__put_user(rL, (unsigned long __user *)rS))
		return -EFAULT;
	rS++;

	aux_reg.usp = (unsigned long)rS;
	current->thread.gpr[10] = (unsigned long)rS;
	current->thread.gpr[11] = (unsigned long)rS;
	current->thread.gpr[20] = newL;
	return 0;
}

static int setup_frame(int sig, struct k_sigaction *ka,
			sigset_t *set, struct pt_regs *regs)
{
	void __user *restorer;
	struct sigframe __user *frame;
	int err = 0;

	frame = get_sigframe(ka, regs, sizeof(*frame));

	if (!access_ok(VERIFY_WRITE, frame, sizeof(*frame)))
		goto badframe;

	err |= __put_user(regs->rBB, &frame->rBB);
	err |= __put_user(regs->rWW, &frame->rWW);
	err |= __put_user(current->thread.gpr[255], &frame->r255);
	err |= __put_user(set->sig[0], &frame->oldmask);
	err |= __put_user(0x00010000 + __NR_sigreturn, frame->retcode + 0);
	/* must be last */
	err |= flush_registerstack(1);

	if (err)
		goto badframe;

	restorer = frame->retcode;
	if (ka->sa.sa_flags & SA_RESTORER)
		restorer = ka->sa.sa_restorer;

	regs->rWW = (unsigned long)ka->sa.sa_handler;
	current->thread.gpr[254] = (unsigned long)frame;
	current->thread.gpr[255] = (unsigned long)restorer;
	current->thread.lr[0] = sig;

	return 0;

badframe:
	printk("setup_frame: badframe\n");
	force_sigsegv(sig, current);
	return -EFAULT;
}

static int
handle_signal(unsigned long sig, siginfo_t *info, struct k_sigaction *ka,
		sigset_t *oldset, struct pt_regs *regs)
{
	int ret;

	if (regs->orig_rBB >= 0) {
		switch (regs->rBB) {
		case -ERESTART_RESTARTBLOCK:
		case -ERESTARTNOHAND:
			printk("handle_signal: abort syscall 1\n");
			regs->rBB = -EINTR;
			break;
		case -ERESTARTSYS:
			if (!(ka->sa.sa_flags & SA_RESTART)) {
				printk("handle_signal: abort syscall 2\n");
				regs->rBB = -EINTR;
				break;
			}
		/* fallthrough */
		case -ERESTARTNOINTR:
			printk("handle_signal: restart syscall\n");
			regs->rBB = regs->orig_rBB;
			regs->rXX &= 0xffffffff;
			break;
		}
	}

	ret = 0;

	if (ka->sa.sa_flags & SA_SIGINFO)
		printk("SA_SIGINFO on\n");
	else
		ret = setup_frame(sig, ka, oldset, regs);

	if (ret == 0) {
		spin_lock_irq(&current->sighand->siglock);
		sigorsets(&current->blocked, &current->blocked, &ka->sa.sa_mask);
		if (!(ka->sa.sa_flags & SA_NODEFER))
			sigaddset(&current->blocked, sig);
		recalc_sigpending();
		spin_unlock_irq(&current->sighand->siglock);
	}

	return ret;
}

void do_signal(struct pt_regs *regs)
{
	siginfo_t info;
	int signr;
	struct k_sigaction ka;
	sigset_t *oldset;

	if (test_thread_flag(TIF_RESTORE_SIGMASK))
		oldset = &current->saved_sigmask;
	else
		oldset = &current->blocked;

	signr = get_signal_to_deliver(&info, &ka, regs, NULL);
	if (signr > 0) {
		if (handle_signal(signr, &info, &ka, oldset, regs) == 0) {
			if (test_thread_flag(TIF_RESTORE_SIGMASK))
				clear_thread_flag(TIF_RESTORE_SIGMASK);
		}
		return;
	}
	if (regs->orig_rBB >= 0) {
		switch (regs->rBB) {
		case -ERESTARTNOHAND:
		case -ERESTARTSYS:
		case -ERESTARTNOINTR:
			regs->rBB = regs->orig_rBB;
			printk("do_signal: retry syscall\n");
			regs->rXX &= 0xffffffff;
			break;
		case -ERESTART_RESTARTBLOCK:
			regs->rBB = regs->orig_rBB;
			printk("do_signal: restart syscall\n");
			regs->rXX &= 0xffff0000;
			regs->rXX |= __NR_restart_syscall;
			break;
		}
	}

	if (test_thread_flag(TIF_RESTORE_SIGMASK)) {
		clear_thread_flag(TIF_RESTORE_SIGMASK);
		sigprocmask(SIG_SETMASK, &current->saved_sigmask, NULL);
	}
}
