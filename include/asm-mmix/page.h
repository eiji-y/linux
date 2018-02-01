/*
 * include/asm-mmix/page.h
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

#ifndef _ASM_MMIX_PAGE_H
#define _ASM_MMIX_PAGE_H

#define PAGE_SHIFT	13
#define PAGE_SIZE	(1UL << PAGE_SHIFT)

#define PAGE_MASK	(~((1 << PAGE_SHIFT) - 1))

#ifdef __KERNEL__

#define	KERNELBASE	0x8000000000000000

#define PTE_SHIFT	(PAGE_SHIFT - 3)	/* 1024 ptes per page */

#define PAGE_OFFSET	KERNELBASE

#define _ALIGN(addr,size)       (((addr)+(size)-1)&(~((size)-1)))

#define PAGE_ALIGN(addr)	_ALIGN(addr, PAGE_SIZE)

#define is_kernel_addr(x)	((x) >= KERNELBASE)

struct page;
#define	clear_page(page)	memset((page), 0, PAGE_SIZE)
extern void copy_page(void *to, void *from);
#define	clear_user_page(page, vaddr, pg)	clear_page(page)
extern void copy_user_page(void *to, void *from, unsigned long vaddr,
			   struct page *pg);

typedef unsigned long pte_t;
#if	0
typedef unsigned long pmd_t;
#endif
typedef unsigned long pgd_t;
typedef unsigned long pgprot_t;

#define pte_val(x)	(x)
#define pgd_val(x)	(x)
#if	0
#define pmd_val(x)	(x)
#define pud_val(x)	(x)
#define __pud(x)	(x)
#endif
#define	__pte(x)	((pte_t) { (x) } )

#define __pa(vaddr) (((unsigned long)(vaddr))-KERNELBASE)
#define __va(paddr) ((void *)((paddr)+KERNELBASE))

#define virt_to_page(kaddr)	pfn_to_page(__pa(kaddr) >> PAGE_SHIFT)

#define pfn_valid(pfn)		((pfn) < max_mapnr)
#define virt_addr_valid(kaddr)	pfn_valid(__pa(kaddr) >> PAGE_SHIFT)

#define VM_DATA_DEFAULT_FLAGS	(VM_READ | VM_WRITE | VM_EXEC | \
				 VM_MAYREAD | VM_MAYWRITE | VM_MAYEXEC)

#define copy_page(to,from)      memcpy((void *)(to), (void *)(from), PAGE_SIZE)

#include <asm-generic/memory_model.h>
#include <asm-generic/page.h>

#endif /* __KERNEL__ */
#endif	/* _ASM_MMIX_PAGE_H */
