/*
 * include/asm-mmix/io.h
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

#ifndef _MMIX_IO_H
#define _MMIX_IO_H
#ifdef __KERNEL__

#include <asm/page.h>

#define IO_SPACE_LIMIT ~0

extern inline void * phys_to_virt(unsigned long address)
{
	return (void *) (address + KERNELBASE);
}

static inline void __raw_writel(__u32 b, volatile void __iomem *addr)
{
	extern void progress(const char * fmt, ...);
	progress("__raw_writel(): Not Yet\n");
}

static inline u8 inb(const volatile void __iomem *addr)
{
	return *(u8 *)addr;
}

static inline void outb(volatile unsigned char __iomem *addr, int val)
{
	__asm__ __volatile__("sync 3; stbu %1,%0,0"
			     :: "r"(addr), "r"(val):"memory");
}

#define page_to_phys(page) (page_to_pfn(page) << PAGE_SHIFT)

#define xlate_dev_kmem_ptr(p)      p
#define xlate_dev_mem_ptr(p)       __va(p)

#endif /* __KERNEL__ */
#endif /* _MMIX_IO_H */
