/*
 * include/asm-mmix/tlbflush.h
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

#ifndef _ASM_MMIX_TLBFLUSH_H
#define _ASM_MMIX_TLBFLUSH_H

static inline void __flush_tlb_pgtables(unsigned long start, unsigned long end)
{
	unsigned long vmaddr;

	for (vmaddr = start; vmaddr < end; vmaddr += PAGE_SIZE) {
		asm volatile ("	ldvts	$255,%0,0"::"r"(vmaddr):"$255", "memory");
	}
}

static inline void flush_tlb_page(struct vm_area_struct *vma,
				unsigned long vmaddr)
{
	if (vma->vm_mm == current->active_mm) {
//		printk("flush_tlb_page(%lx)\n", vmaddr);
		vmaddr &= 0xffffffffffffe000;
		asm volatile ("	ldvts	$255,%0,0"::"r"(vmaddr):"$255", "memory");
	}
}

static inline void flush_tlb_mm(struct mm_struct *mm)
{
	if (mm == current->active_mm) {
		struct vm_area_struct * mmap;

//		printk("flush_tlb_mm(%p): Not Yet\n", mm);

		for (mmap = mm->mmap; mmap; mmap = mmap->vm_next) {
			__flush_tlb_pgtables(mmap->vm_start, mmap->vm_end);
		}
	}
}

static inline void flush_tlb_pgtables(struct mm_struct *mm,
		unsigned long start, unsigned long end)
{
	if (mm == current->active_mm) {
//		printk("flush_tlb_pgtables(%p, %lx, %lx): Not Yet\n", mm, start, end);
		__flush_tlb_pgtables(start, end);
	}
}

static inline void flush_tlb_kernel_range(unsigned long start,
		unsigned long end)
{
	printk("flush_tlb_kernel_range(): Not Yet\n");
}

static inline void flush_tlb_range(struct vm_area_struct *vma,
	unsigned long start, unsigned long end)
{
	printk("flush_tlb_range(): Not Yet\n");
}

#endif /* _ASM_MMIX_TLBFLUSH_H */
