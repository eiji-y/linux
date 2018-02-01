/*
 * include/asm-mmix/cacheflush.h
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

#ifndef _ASM_MMIX_CACHEFLUSH_H
#define _ASM_MMIX_CACHEFLUSH_H

#ifdef __KERNEL__

#define flush_cache_mm(mm)			do { } while (0)
#define flush_cache_range(vma, start, end)	do { } while (0)
#define flush_cache_page(vma, vmaddr, pfn)	do { } while (0)
#define flush_icache_page(vma, page)		do { } while (0)

extern void flush_dcache_page(struct page *page);
extern void flush_icache_range(unsigned long start, unsigned long stop);

#define flush_dcache_mmap_lock(mapping)		do { } while (0)
#define flush_dcache_mmap_unlock(mapping)	do { } while (0)

#define flush_cache_vmap(start, end)          do { } while (0)
#define flush_cache_vunmap(start, end)        do { } while (0)

#define copy_to_user_page(vma, page, vaddr, dst, src, len) \
	do { \
		extern void progress(const char * fmt, ...); \
		progress("copy_to_user_page(): Not Yet\n"); \
	} while (0)
#define copy_from_user_page(vma, page, vaddr, dst, src, len) \
	do { \
		extern void progress(const char * fmt, ...); \
		progress("copy_from_user_page(): Not Yet\n"); \
	} while (0)


#endif /* __KERNEL__ */

#endif /* _ASM_MMIX_CACHEFLUSH_H */
