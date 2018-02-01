/*
 * include/asm-mmix/pgalloc.h
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

#ifndef _MMIX_PGALLOC_H
#define _MMIX_PGALLOC_H

#include <linux/mm.h>

extern pgd_t *pgd_alloc(struct mm_struct *mm);
extern void pgd_free(struct mm_struct *mm);

//#define pmd_alloc_one(mm,address)       ({ BUG(); ((pmd_t *)2); })
#if	0
static inline pmd_t *pmd_alloc_one(struct mm_struct *mm, unsigned long addr)
{
	return (pmd_t *)__get_free_page(GFP_KERNEL|__GFP_ZERO);
}

//#define pgd_populate(mm, pmd, pte)      BUG()
static inline void pgd_populate(struct mm_struct *mm, pgd_t *pgd, pmd_t *pmd)
{
	*pgd = (pgd_t)((unsigned long)pmd | 0x8000000000000000);
}
#endif

static inline void pte_free(struct page *ptepage)
{
//	extern void progress(const char * fmt, ...);
//	progress("pte_free(): Not Yet\n");
}

#if	0
static inline void pmd_free(pmd_t *pmd)
{
	extern void progress(const char * fmt, ...);
	progress("pmd_free(): Not Yet\n");
}
#endif

#define pmd_populate_kernel(mm, pmd, pte)	\
		(pmd_val(*(pmd)) = (unsigned long)pte | _PMD_PRESENT)
#define pmd_populate(mm, pmd, pte)	\
		(pmd_val(*(pmd)) = (unsigned long)lowmem_page_address(pte) | _PMD_PRESENT)

extern pte_t *pte_alloc_one_kernel(struct mm_struct *mm, unsigned long addr);
extern struct page *pte_alloc_one(struct mm_struct *mm, unsigned long addr);
extern void pte_free_kernel(pte_t *pte);

#define __pte_free_tlb(tlb, pte)	pte_free((pte))
#if	0
#define __pmd_free_tlb(tlb, pmd) 	pmd_free((pmd))
#endif

#define check_pgt_cache()	do { } while (0)

#endif /* _MMIX_PGALLOC_H */
