/*
 * include/asm-mmix/tlb.h
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

#ifndef _ASM_MMIX_TLB_H
#define _ASM_MMIX_TLB_H
#ifdef __KERNEL__

#include <asm/pgalloc.h>
#include <asm/tlbflush.h>

#define tlb_start_vma(tlb, vma)	do { } while (0)
#define tlb_end_vma(tlb, vma)	do { } while (0)

#define tlb_flush(tlb)			flush_tlb_mm((tlb)->mm)

#define __tlb_remove_tlb_entry(tlb, pte, address) do { } while (0)

#include <asm-generic/tlb.h>

#endif /* __KERNEL__ */
#endif /* __ASM_MMIX_TLB_H */
