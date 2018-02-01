/*
 * arch/mmix/kernel/process.c
 *
 *   Copyright (C) 2008-2009,2011 Eiji Yoshiya (eiji-y@pb3.so-net.ne.jp)
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
#include <linux/ptrace.h>
#include <linux/fs.h>
#include <linux/elf.h>
#include <linux/mm.h>
#include <linux/binfmts.h>
#include <linux/module.h>
#include <linux/kallsyms.h>
#include <asm/reg.h>
#include <asm/uaccess.h>

void cpu_idle(void)
{
	while (1) {
		while (!need_resched()) {
			cpu_relax();
		}
		preempt_enable_no_resched();
		schedule();
		preempt_disable();
	}
}

static void kernel_thread_helper(int (*fn)(void *), void *arg, struct task_struct *prev)
{
	extern asmlinkage void schedule_tail(struct task_struct *prev);
	int	rval;

//	printk("kernel_thread_helper()\n");
//	printk("fn = %p, arg = %p, prev = %p\n", fn, arg, prev);

	schedule_tail(prev);
	rval = (*fn)(arg);
	do_exit(rval);
}

long kernel_thread(int (*fn)(void *), void * arg, unsigned long flags)
{
	struct pt_regs regs;

//	printk("kernel_thread(): Not Yet\n");
//	printk("fn = %p, arg = %p\n", fn, arg);

	memset(&regs, 0, sizeof(regs));
	regs.rJ = (unsigned long)kernel_thread_helper;
	regs.r0 = (unsigned long)fn;
	regs.r1 = (unsigned long)arg;

	return do_fork(flags | CLONE_VM | CLONE_UNTRACED,
			0, &regs, 0, NULL, NULL);
}

void exit_thread(void)
{
//	printk("exit_thread(): Waht to do???\n");
}

void flush_thread(void)
{
//	printk("flush_thread(): Waht to do???\n");
}

void release_thread(struct task_struct *dead_task)
{
//	printk("release_thread(): Waht to do???\n");
}

void prepare_to_copy(struct task_struct *tsk)
{
//	printk("prepare_to_copy(): What to do???\n");
}

int copy_thread(int nr, unsigned long clone_flags, unsigned long usp,
	unsigned long unused,
	struct task_struct * p, struct pt_regs * regs)
{
	extern void ret_from_syscall(void);
	unsigned long *hw_stack = (unsigned long *)
		((unsigned long)((p->thread_info + 1) + 7) & ~7);
	unsigned long rG;
	unsigned long i;

//	printk("copy_thread(): usp = %lx, regs = %p\n", usp, regs);

	/* copy user context */
	memcpy(p->thread.gpr, current->thread.gpr, sizeof(p->thread.gpr));
	memcpy(p->thread.lr, current->thread.lr, sizeof(p->thread.lr));
	p->thread.gpr[254] = usp;

	rG = getspr(rG);

	if (usp) {
		*hw_stack++ = 0;		/* rL for pop */
	} else {
		*hw_stack++ = regs->r0;		/* r0 for pop */
		*hw_stack++ = regs->r1;		/* r1 for pop */
		*hw_stack++ = 2;		/* rL for pop */
	}
	*hw_stack++ = 0;		/* rL for unsave */
	for (i = rG; i < 254; i++)
		*hw_stack++ = 0;	/* Gn */
	*hw_stack++ =
		(unsigned long)(p->thread_info) + THREAD_SIZE - 8;/* r254 = sp */
	*hw_stack++ = 0;		/* r255 */
	*hw_stack++ = 0;		/* rB ret value*/
	*hw_stack++ = 0;		/* rD */
	*hw_stack++ = 0;		/* rE */
	*hw_stack++ = 0;		/* rH */
	if (usp)
		*hw_stack++ = (unsigned long)ret_from_syscall;
	else
		*hw_stack++ = regs->rJ;	/* rJ */
	*hw_stack++ = 0;		/* rM */
	*hw_stack++ = 0;		/* rR */
	*hw_stack++ = 0;		/* rP */
	*hw_stack++ = regs->rWW;		/* rW */
	*hw_stack++ = regs->rXX;		/* rX */
	*hw_stack++ = regs->rYY;		/* rY */
	*hw_stack++ = regs->rZZ;		/* rZ */
	*hw_stack = rG << 56;		/* rG & rA */

	p->thread.ksp = (unsigned long)hw_stack;

	return 0;
}

struct task_struct fastcall * __switch_to(struct task_struct *prev_p, struct task_struct *next_p)
{
	struct	thread_struct *next_thread, *prev_thread;
	struct	task_struct *prev;
	unsigned long flags;

//	printk("__switch_to(): Not Yet\n");
//	printk("prev_p = %p, next_p = %p\n", prev_p, next_p);

	prev_thread = &prev_p->thread;
	next_thread = &next_p->thread;

	local_irq_save(flags);

	aux_reg.ucp = __pa(next_thread->gpr);
	prev = _switch(prev_thread, next_thread);

	local_irq_restore(flags);
//	printk("prev_p = %p\n", prev);
	return prev;
}

int sys_clone(unsigned long clone_flags, unsigned long usp,
	      int __user *parent_tidp, void __user *child_threadptr,
	      int __user *child_tidp, int p6,
	      struct pt_regs *regs)
{
	if (usp == 0)
		usp = current->thread.gpr[254];
 	return do_fork(clone_flags, usp, regs, 0, parent_tidp, child_tidp);
}
/*
 * sys_execve() executes a new program.
 */
int sys_execve(unsigned long a0, unsigned long a1, unsigned long a2,
               unsigned long a3, unsigned long a4, unsigned long a5,
               struct pt_regs *regs)
{
	int error;
	char * filename;

	filename = getname((char __user *) a0);
	error = PTR_ERR(filename);
	if (IS_ERR(filename))
		goto out;
	error = do_execve(filename,
			(char __user * __user *) a1,
			(char __user * __user *) a2,
			regs);
	if (error == 0) {
		task_lock(current);
		current->ptrace &= ~PT_DTRACE;
		task_unlock(current);
	}
	putname(filename);
out:
	return error;
}

int execve(const char *file, char **argv, char **envp)
{
	int error;
	struct pt_regs regs;
	void *args[3];

#if	0
	char **ppc;

	printk("execve():\n");
	printk("file = %s\n", file);
	printk("argv = \n");
	for (ppc = argv; *ppc; ppc++)
		printk("\t%s\n", *ppc);
	printk("envp = \n");
	for (ppc = envp; *ppc; ppc++)
		printk("\t%s\n", *ppc);
#endif

	args[0] = (void *)file;
	args[1] = (void *)argv;
	args[2] = (void *)envp;
	regs.r255 = (unsigned long)args;

	error = sys_execve(
			(unsigned long)file, (unsigned long)argv,
			(unsigned long)envp, (unsigned long)0,
			(unsigned long)0, (unsigned long)0,
			&regs);
	if (error == 0) {
		extern void syscall_ret(unsigned long usp);
		struct task_struct * p = get_current();
		unsigned long *hw_stack = (unsigned long *)
			((unsigned long)((p->thread_info + 1) + 7) & ~7);
		unsigned long rG;
		unsigned long i;

		putspr(rK, 0);
		rG = getspr(rG);

		// save	(kernel stack)
		*hw_stack++ = 0;		/* rL for unsave */
		for (i = rG; i < 254; i++)
			*hw_stack++ = 0;	/* Gn */
		*hw_stack++ =
			(unsigned long)(p->thread_info) + THREAD_SIZE - 8;/* r254 = sp */
		*hw_stack++ = 0;		/* r255 */
		*hw_stack++ = 0;		/* rB */
		*hw_stack++ = 0;		/* rD */
		*hw_stack++ = 0;		/* rE */
		*hw_stack++ = 0;		/* rH */
		*hw_stack++ = 0;		/* rJ */
		*hw_stack++ = 0;		/* rM */
		*hw_stack++ = 0;		/* rR */
		*hw_stack++ = 0;		/* rP */
		*hw_stack++ = 0;		/* rW */
		*hw_stack++ = 0;		/* rX */
		*hw_stack++ = 0;		/* rY */
		*hw_stack++ = 0;		/* rZ */
		*hw_stack = rG << 56;		/* rG & rA */

		aux_reg.ksp = p->thread.ksp = (unsigned long)hw_stack;

//printk("jump to user: ksp = %lx\n", aux_reg.ksp);
		putspr(rBB, regs.rBB);
		putspr(rWW, regs.rWW);
		putspr(rXX, regs.rXX);
		putspr(rYY, regs.rYY);
		putspr(rZZ, regs.rZZ);

//printk("sys_execve(): preempt_count = %d\n", preempt_count());
		syscall_ret(__pa(p->thread.gpr));

		// NOT REACHED
	}
//	printk("execve(): error = %d\n", error);
	return error;
}

/* setup gpr */
static int setup_gpr(struct linux_binprm *bprm)
{
	struct elfhdr *elf_ex;
	struct elf64_shdr *elf_shdr = NULL;
	struct elf64_shdr *elf_shstr;
	unsigned char *shstr = NULL;
	int	i;
	int	retval = 0;
	unsigned long size;

	elf_ex = (struct elfhdr *)bprm->buf;

#if	0
	printk("e_shoff = %lx\n", elf_ex->e_shoff);
	printk("e_shentsize = %d\n", elf_ex->e_shentsize);
	printk("e_shnum = %x\n", elf_ex->e_shnum);
	printk("e_shstrndx = %x\n", elf_ex->e_shstrndx);
#endif
	if (elf_ex->e_shstrndx >= elf_ex->e_shnum) {
		retval = -ENOEXEC;
		goto out_ret;
	}

	size = elf_ex->e_shentsize * elf_ex->e_shnum;

	elf_shdr = kmalloc(size, GFP_KERNEL);
	if (!elf_shdr) {
		retval = -ENOMEM;
		goto out_ret;
	}
	retval = kernel_read(bprm->file, elf_ex->e_shoff,
			     (char *)elf_shdr, size);
#if	0
	printk("retval = %d\n", retval);
#endif
	if (retval != size) {
		if (retval >= 0)
			retval = -EIO;
		goto out_ret;
	}

	elf_shstr = elf_shdr + elf_ex->e_shstrndx;
	shstr = kmalloc(elf_shstr->sh_size, GFP_KERNEL);
	if (!shstr) {
		retval = -ENOMEM;
		goto out_ret;
	}
	retval = kernel_read(bprm->file, elf_shstr->sh_offset,
			     (char *)shstr, elf_shstr->sh_size);
#if	0
	printk("retval = %d\n", retval);
#endif
	if (retval != elf_shstr->sh_size) {
		if (retval >= 0)
			retval = -EIO;
		goto out_ret;
	}

	for (i = 0; i < elf_ex->e_shnum; i++) {
#if	0
		printk("section %d\n", i);
		printk("  sh_name = %s\n", shstr + elf_shdr[i].sh_name);
		printk("  sh_type = %x\n", elf_shdr[i].sh_type);
		printk("  sh_flags = %lx\n", elf_shdr[i].sh_flags);
		printk("  sh_addr = %lx\n", elf_shdr[i].sh_addr);
		printk("  sh_offset = %lx\n", elf_shdr[i].sh_offset);
		printk("  sh_size = %lx\n", elf_shdr[i].sh_size);
		printk("  sh_link = %x\n", elf_shdr[i].sh_link);
		printk("  sh_info = %x\n", elf_shdr[i].sh_info);
		printk("  sh_addralign = %lx\n", elf_shdr[i].sh_addralign);
		printk("  sh_entsize = %lx\n", elf_shdr[i].sh_entsize);
#endif
		if (strcmp(shstr+elf_shdr[i].sh_name, ".MMIX.reg_contents") == 0) {
			struct task_struct * p = get_current();

			if (elf_shdr[i].sh_addr + elf_shdr[i].sh_size != 0x7f8) {
				retval = -ENOMEM;
				goto out_ret;
			}
			retval = kernel_read(bprm->file,
				elf_shdr[i].sh_offset,
				(char *)p->thread.gpr + elf_shdr[i].sh_addr,
				elf_shdr[i].sh_size);
			if (retval != elf_shdr[i].sh_size) {
				if (retval >= 0)
					retval = -EIO;
			}
			retval = 0;
			goto out_ret;
		}
	}
	retval = -ENOEXEC;
out_ret:
	if (shstr)
		kfree(shstr);
	if (elf_shdr)
		kfree(elf_shdr);
#if	0
	printk("retval = %d\n", retval);
#endif
	return retval;
}

/* Setup a VMA for register stack */
int arch_setup_additional_pages(struct linux_binprm *bprm, int exstack)
{
	struct vm_area_struct *vma;
	struct mm_struct *mm = current->mm;
	int ret = 0;

//	printk("arch_setup_additional_pages\n");
	ret = setup_gpr(bprm);
	if (ret < 0)
		return ret;

	vma = kmem_cache_zalloc(vm_area_cachep, SLAB_KERNEL);
	if (!vma)
		return -ENOMEM;

	down_write(&mm->mmap_sem);

	vma->vm_mm = mm;
	vma->vm_start = 0x6000000000000000;
	vma->vm_end = vma->vm_start + PAGE_SIZE;
	vma->vm_flags = VM_DATA_DEFAULT_FLAGS | VM_ACCOUNT | mm->def_flags;
	vma->vm_page_prot = protection_map[vma->vm_flags &
				(VM_READ|VM_WRITE|VM_EXEC|VM_SHARED)];

	if ((ret = insert_vm_struct(mm, vma))) {
		kmem_cache_free(vm_area_cachep, vma);
		goto up_ret;
	}
	mm->total_vm++;
up_ret:
	up_write(&mm->mmap_sem);
	return ret;
}

/////

unsigned long get_wchan(struct task_struct *p)
{
	extern void progress(const char * fmt, ...);
	progress("get_wchan(): Not Yet\n");
	return 0;
}

void show_regs(struct pt_regs *regs)
{
	extern void progress(const char * fmt, ...);
	progress("sho_regs(): Not Yet\n");
}

void start_thread(struct pt_regs *regs, unsigned long start, unsigned long sp)
{
	unsigned long *hw_stack = (unsigned long *)0x6000000000000000;
#if	0
	int i;
	unsigned long rG;
#endif
	struct task_struct * p = get_current();

#if	0
	unsigned long *usp = (unsigned long *)sp;
	printk("start_thread(regs = %p, start = %016lx, sp = %016lx): Net Yet\n", regs, start, sp);
	printk("sp[0] = %lx\n", usp[0]);
	printk("sp[1] = %lx\n", usp[1]);
	printk("sp[2] = %lx\n", usp[2]);
	printk("sp[3] = %lx\n", usp[3]);
	printk("sp[4] = %lx\n", usp[4]);
	printk("sp[5] = %lx\n", usp[5]);

	printk("argv = &sp[1] = %lx\n", &usp[1]);
	printk("envp = &sp[1] = %lx\n", &usp[3]);
#endif

	p->thread.gpr[rL] = 0;
	p->thread.gpr[rB] = 0;
	p->thread.gpr[rD] = 0;
	p->thread.gpr[rE] = 0;
	p->thread.gpr[rH] = 0;
	p->thread.gpr[rJ] = 0;
	p->thread.gpr[rM] = 0;
	p->thread.gpr[rR] = 0;
	p->thread.gpr[rP] = 0;
	p->thread.gpr[rW] = 0;
	p->thread.gpr[rX] = 0;
	p->thread.gpr[rY] = 0;
	p->thread.gpr[rZ] = 0;
	p->thread.gpr[rA] = 0;
	p->thread.gpr[rG] = 0x20;
	p->thread.gpr[rS] = (unsigned long)hw_stack;
	p->thread.gpr[rO] = (unsigned long)hw_stack;

	p->thread.gpr[254] = sp;
	p->thread.gpr[255] = 0;

	regs->rXX = 0x8000000000000000;
	regs->rWW = start;
}

int sys_fork(unsigned long p1, unsigned long p2, unsigned long p3,
	     unsigned long p4, unsigned long p5, unsigned long p6,
	     struct pt_regs *regs)
{
	printk("sys_fork(): Not Yet\n");
	for (;;)
		;
	return 0;
}

int sys_vfork(unsigned long p1, unsigned long p2, unsigned long p3,
	      unsigned long p4, unsigned long p5, unsigned long p6,
	      struct pt_regs *regs)
{
	printk("sys_vfork(): Not Yet\n");
	for (;;)
		;
	return 0;
}

void dump_stack(void)
{
	unsigned long *sp;
	struct task_struct * p = current;
	unsigned long *hw_stack = (unsigned long *)
		((unsigned long)((p->thread_info + 1) + 7) & ~7);

//	show_stack(current, NULL);
	__asm__ __volatile__(
	"save	$255,0\n\
	unsave	0,$255\n\
	get	%0,rO":"=r"(sp));

	print_symbol("caller -> %s\n", getspr(rJ));

	printk("hw stack:\n");
	while (hw_stack < sp) {
		long frame_size;

		frame_size = (long)*--sp;
		printk("%p: frame size = %ld\n", sp, frame_size);
		if (frame_size > 255) {
			printk("invalid frame size\n");
			break;
		}
		while (frame_size > 0) {
			unsigned long	reg = *--sp;

			printk("$%ld: 0x%016lx", --frame_size, reg);
			if ((reg & 0xffffffff00000000) == 0x8000000000000000) {
				print_symbol(" -> %s", reg);
			}
			printk("\n");
		}
	}
}
EXPORT_SYMBOL(dump_stack);
