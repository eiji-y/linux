/*
 * include/asm-mmix/pgtable.h
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

#ifndef _MMIX_PGTABLE_H
#define _MMIX_PGTABLE_H

#include <asm/page.h>
#if	0
#include <asm-generic/4level-fixup.h>
#undef	pmd_alloc
#else
#include <asm-generic/pgtable-nopmd.h>
#endif

#include <linux/sched.h>
struct mm_struct;

extern pgd_t swapper_pg_dir[1024];

extern unsigned long ioremap_bot, ioremap_base;

#if	0
#define PMD_SHIFT	(PAGE_SHIFT + PTE_SHIFT)
#define PMD_SIZE	(1UL << PMD_SHIFT)
#define PMD_MASK	(~(PMD_SIZE-1))
#endif

#define PGDIR_SHIFT	PAGE_SHIFT
#define PGDIR_SIZE	(1UL << PGDIR_SHIFT)
#define PGDIR_MASK	(~(PGDIR_SIZE-1))

#define _PAGE_PRESENT	0x8000000000000000
#define _PAGE_DIRTY	0x4000000000000000
#define _PAGE_ACCESSED	0x2000000000000000
#define _PAGE_FILE	0x1000000000000000
#define _PAGE_USER	0x0800000000000000
#define _PAGE_READ	0x0000000000000004
#define _PAGE_WRITE	0x0000000000000002
#define _PAGE_EXEC	0x0000000000000001

#define PAGE_NONE		(_PAGE_PRESENT|_PAGE_ACCESSED)
#define PAGE_READONLY		(_PAGE_PRESENT|_PAGE_ACCESSED|_PAGE_READ)
#define PAGE_COPY_NOREAD	(_PAGE_PRESENT|_PAGE_ACCESSED)
#define PAGE_COPY		(_PAGE_PRESENT|_PAGE_ACCESSED|_PAGE_READ)
#define PAGE_SHARED_NOREAD	(_PAGE_PRESENT|_PAGE_ACCESSED|_PAGE_WRITE)
#define PAGE_SHARED		(_PAGE_PRESENT|_PAGE_ACCESSED|_PAGE_READ|_PAGE_WRITE)
#define PAGE_EXECONLY		(_PAGE_PRESENT|_PAGE_ACCESSED|_PAGE_EXEC)
#define PAGE_READONLY_X		(_PAGE_PRESENT|_PAGE_ACCESSED|_PAGE_READ|_PAGE_EXEC)
#define PAGE_COPY_NOREAD_X	(_PAGE_PRESENT|_PAGE_ACCESSED|_PAGE_EXEC)
#define PAGE_COPY_X		(_PAGE_PRESENT|_PAGE_ACCESSED|_PAGE_READ|_PAGE_EXEC)
#define PAGE_SHARED_NOREAD_X	(_PAGE_PRESENT|_PAGE_ACCESSED|_PAGE_WRITE|_PAGE_EXEC)
#define PAGE_SHARED_X		(_PAGE_PRESENT|_PAGE_ACCESSED|_PAGE_READ|_PAGE_WRITE|_PAGE_EXEC)

#define _PMD_PRESENT	0x8000000000000000
#define _PMD_PRESENT_MASK (PAGE_MASK)

#define FIRST_USER_ADDRESS	0

#define __P000	PAGE_NONE
#define __P001	PAGE_READONLY
#define __P010	PAGE_COPY_NOREAD
#define __P011	PAGE_COPY
#define __P100	PAGE_EXECONLY
#define __P101	PAGE_READONLY_X
#define __P110	PAGE_COPY_NOREAD_X
#define __P111	PAGE_COPY_X

#define __S000	PAGE_NONE
#define __S001	PAGE_READONLY
#define __S010	PAGE_SHARED_NOREAD
#define __S011	PAGE_SHARED
#define __S100	PAGE_EXECONLY
#define __S101	PAGE_READONLY_X
#define __S110	PAGE_SHARED_NOREAD_X
#define __S111	PAGE_SHARED_X

#define PAGE_KERNEL	0

#define PFN_SHIFT_OFFSET	(PAGE_SHIFT)
#define PTE_RPN_SHIFT       (PAGE_SHIFT)

#define pte_pfn(x)		((pte_val(x) >> PFN_SHIFT_OFFSET) & 0xffffffffffff)
#define pte_present(pte)	(pte_val(pte) & _PAGE_PRESENT)
#define pte_none(pte)		(pte_val(pte) == 0)
#define pte_clear(mm,addr,ptep)	set_pte_at((mm),(addr),(ptep),__pte(0))
#define pte_unmap(pte)			do { } while(0)
#define pte_unmap_nested(pte)	do { } while (0)
#define pfn_pte(pfn, prot)	((pfn<<13)|(prot))
#define mk_pte(page, prot)	pfn_pte(page_to_pfn(page),(prot))
#define pgoff_to_pte(off)	((pte_t) {((off) << PTE_RPN_SHIFT)|_PAGE_FILE})
#define PTE_FILE_MAX_BITS	(BITS_PER_LONG - PTE_RPN_SHIFT)
#define pte_to_pgoff(pte)	(pte_val(pte) >> 3)

#define	pmd_clear(pmdp)		do { pmd_val(*(pmdp)) = 0; } while (0)
#define pmd_none(pmd)		(!pmd_val(pmd))
#define	pmd_bad(pmd)		0
#define	pmd_present(pmd)	1

#if	0
extern pmd_t *pmd_alloc(struct mm_struct *mm, pgd_t *pgd, unsigned long addr);
extern pmd_t *pmd_offset(pud_t *pud, unsigned long addr);
#endif

#define pte_ERROR(e) \
	printk("%s:%d: bad pte %08lx.\n", __FILE__, __LINE__, pte_val(e))
#if	0
#define pmd_ERROR(e) \
	printk("%s:%d: bad pmd %08lx.\n", __FILE__, __LINE__, pmd_val(e))
#endif
#define pgd_ERROR(e) \
	printk("%s:%d: bad pgd %08lx.\n", __FILE__, __LINE__, pgd_val(e))

#define pgd_clear(xp)				do { } while (0)

#define pmd_page_kernel(pmd)	\
	((unsigned long) (pmd_val(pmd) & PAGE_MASK))
#define pmd_page(pmd)		\
	(mem_map + (__pa(pmd_val(pmd)) >> PAGE_SHIFT))

#define pgd_index(address)	 ((address) >> PGDIR_SHIFT)
//#define pgd_offset(mm, address)	 ((mm)->pgd + pgd_index(address))
extern pgd_t *pgd_offset(struct mm_struct *mm, unsigned long address);
#define pgd_offset_k(address) pgd_offset(&init_mm, address)

static inline int pte_read(pte_t pte)		{ return pte_val(pte) & _PAGE_USER; }
static inline int pte_write(pte_t pte)		{ return pte_val(pte) & _PAGE_WRITE; }
static inline int pte_young(pte_t pte)		{ return pte_val(pte) & _PAGE_ACCESSED; }
static inline int pte_dirty(pte_t pte)		{ return pte_val(pte) & _PAGE_DIRTY; }

static inline pte_t pte_mkread(pte_t pte) {
	pte_val(pte) |= _PAGE_USER; return pte; }
static inline pte_t pte_mkexec(pte_t pte) {
	pte_val(pte) |= _PAGE_USER | _PAGE_EXEC; return pte; }
static inline pte_t pte_mkwrite(pte_t pte) {
	pte_val(pte) |= _PAGE_WRITE; return pte; }
static inline pte_t pte_mkdirty(pte_t pte) {
	pte_val(pte) |= _PAGE_DIRTY; return pte; }
static inline pte_t pte_mkyoung(pte_t pte) {
	pte_val(pte) |= _PAGE_ACCESSED; return pte; }

#if	0
static inline int pgd_none(pgd_t pgd)		{ return 0; }
static inline int pgd_bad(pgd_t pgd)		{ return 0; }
static inline int pgd_present(pgd_t pgd)	{ return pgd & 0x8000000000000000; }
#endif

#define pte_page(x)		pfn_to_page(pte_pfn(x))

#define VMALLOC_OFFSET (0x1000000) /* 16M */
#define VMALLOC_START ((((long)high_memory + VMALLOC_OFFSET) & ~(VMALLOC_OFFSET-1)))
#define VMALLOC_END	ioremap_bot

#define kern_addr_valid(addr)        (1)

static inline int pte_file(pte_t pte)		{ return pte_val(pte) & _PAGE_FILE; }
static inline pte_t pte_wrprotect(pte_t pte) {
	pte_val(pte) &= ~(_PAGE_WRITE); return pte; }

extern unsigned long empty_zero_page[1024];
#define ZERO_PAGE(vaddr) (virt_to_page(empty_zero_page))

#if	0
static inline pte_t *pte_offset_map(pmd_t *pmd, unsigned long addr)
{
	return (pte_t *)pmd + ((addr >> 13) & 0x3ff);
}

static inline pte_t *pte_offset_map_nested(pmd_t *pmd, unsigned long addr)
{
	return (pte_t *)pmd + ((addr >> 13) & 0x3ff);
}
#else
extern pte_t *pte_offset_map(pmd_t *pmd, unsigned long addr);
#define	pte_offset_map_nested(dir, addr)	pte_offset_map((dir), (addr))
#endif

static inline void set_pte_at(struct mm_struct *mm, unsigned long addr,
			      pte_t *ptep, pte_t pte)
{
	*ptep = pte;
}

static inline pte_t pte_mkclean(pte_t pte) {
	pte_val(pte) &= ~(_PAGE_DIRTY); return pte; }
static inline pte_t pte_mkold(pte_t pte) {
	pte_val(pte) &= ~_PAGE_ACCESSED; return pte; }
static inline pte_t pte_modify(pte_t pte, pgprot_t newprot)
{
	extern void progress(const char * fmt, ...);
	progress("pte_modify(): Not Yet\n");
	return 0;
}


#define __swp_type(entry)		((entry).val & 0x1f)
#define __swp_offset(entry)		((entry).val >> 5)
#define __swp_entry(type, offset)	((swp_entry_t) { (type) | ((offset) << 5) })
#define __pte_to_swp_entry(pte)		((swp_entry_t) { pte_val(pte) >> 3 })
#define __swp_entry_to_pte(x)		((pte_t) { (x).val << 3 })

#define pgtable_cache_init()	do { } while (0)
#define PTRS_PER_PTE	(1 << PTE_SHIFT)
#define pte_index(address)		\
	(((address) >> PAGE_SHIFT) & (PTRS_PER_PTE - 1))
#define pte_offset_kernel(dir, addr)	\
	((pte_t *) pmd_page_kernel(*(dir)) + pte_index(addr))

#define update_mmu_cache(vma,address,pte) do { } while (0)

#include <asm-generic/pgtable.h>

#if	0
#undef	pgd_addr_end
#define	pgd_addr_end(addr, end)	(end)
#endif

#endif /* _MMIX_PGTABLE_H */
