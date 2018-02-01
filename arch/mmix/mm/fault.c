#include <linux/ptrace.h>
#include <linux/mm.h>
#include <linux/rmap.h>
#include <linux/hardirq.h>
#include <asm/current.h>

int expand_register_stack(struct mm_struct *mm, unsigned long address)
{
	struct vm_area_struct *vma;

	vma = find_vma(mm, 0x6000000000000000);
	if (!vma) {
		printk("register stack not found\n");
		return -ENOMEM;
	}
	printk("vma->vm_end = %lx\n", vma->vm_end);
	if (address != vma->vm_end)
		return -ENOMEM;

	if (unlikely(anon_vma_prepare(vma)))
		return -ENOMEM;
	anon_vma_lock(vma);

	vma->vm_end += PAGE_SIZE;

	anon_vma_unlock(vma);
	return 0;
}

/*
 * code: 4 = read
 * code: 2 = write
 * code: 1 = exec
 */
void do_page_fault(struct pt_regs *regs, int error_code)
{
	struct vm_area_struct *vma;
	struct mm_struct *mm = current->mm;
	unsigned long address;
	int code = SEGV_MAPERR;

#if	0
	printk("page_fault() pid = %d\n", current->pid);
	printk("error_code = %x\n", error_code);
	printk("mm = %p\n", mm);
	printk("pc = %lx, ", regs->rWW);
	printk("address = %lx\n", regs->rYY);
#endif

	address = regs->rYY;

//printk("do_page_fault(): preempt_count = %d\n", preempt_count());
	if (in_atomic() || !mm)
		goto bad_area_nosemaphore;
	down_read(&mm->mmap_sem);
	vma = find_vma(mm, address);
//	printk("vma = %p\n", vma);
	if (!vma)
		goto bad_area;
//	printk("vma->vm_start = %lx\n", vma->vm_start);
	if (vma->vm_start <= address)
		goto good_area;
	if (!(vma->vm_flags & VM_GROWSDOWN))
		goto bad_area;
	/* stack */
	/* check regsiter stack */
	printk("stack expansion\n");
	printk("pc = %lx\n", regs->rWW);
	printk("address = %lx\n", regs->rYY);
	if (expand_register_stack(mm, address) && expand_stack(vma, address))
		goto bad_area;

good_area:
	code = SEGV_ACCERR;
	switch (error_code) {
	case 1:
		if (!(vma->vm_flags & VM_EXEC))
			goto bad_area;
		break;
	case 2:
		if (!(vma->vm_flags & VM_WRITE))
			goto bad_area;
		break;
	case 4:
		if (!(vma->vm_flags & VM_READ))
			goto bad_area;
		break;
	default:
		goto bad_area;
	}

	switch (handle_mm_fault(mm, vma, address, error_code & 2)) {
	case VM_FAULT_MINOR:
		current->min_flt++;
		break;
	case VM_FAULT_MAJOR:
		current->maj_flt++;
		break;
	case VM_FAULT_SIGBUS:
		goto do_sigbus;
	case VM_FAULT_OOM:
		goto out_of_memory;
	default:
		BUG();
	}
	up_read(&mm->mmap_sem);
//printk("do_page_fault(): done\n");
	return;

bad_area:
	printk("bad_area: NOT YET\n");
	printk("pc = %lx\n", regs->rWW);
	printk("address = %lx\n", regs->rYY);
	printk("vma = %p\n", vma);
	up_read(&mm->mmap_sem);
	for (;;)
		;
bad_area_nosemaphore:
	printk("bad_area_nosemaphore: NOT YET\n");
	for (;;)
		;
do_sigbus:
	printk("do_sigbus: NOT YET\n");
	for (;;)
		;
out_of_memory:
	printk("out_of_memory: NOT YET\n");
	for (;;)
		;

}
