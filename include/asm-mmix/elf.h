/*
 * include/asm-mmix/elf.h
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

#ifndef _ASM_MMIX_ELF_H
#define _ASM_MMIX_ELF_H

#define ELF_NGREG	48	/* XXX */
#define ELF_NFPREG	33	/* XXX */

#define ELF_CLASS	ELFCLASS64

typedef unsigned long elf_greg_t;
typedef elf_greg_t elf_gregset_t[ELF_NGREG];

typedef double elf_fpreg_t;
typedef elf_fpreg_t elf_fpregset_t[ELF_NFPREG];

#ifdef __KERNEL__

#define elf_check_arch(x) ((x)->e_machine == EM_MMIX)

#define ELF_EXEC_PAGESIZE	PAGE_SIZE

#define ELF_ET_DYN_BASE         (0x08000000)	/* XXX */

#define ARCH_HAS_SETUP_ADDITIONAL_PAGES
struct linux_binprm;
extern int arch_setup_additional_pages(struct linux_binprm *bprm,
                                       int executable_stack);

#endif /* __KERNEL__ */

#define ELF_HWCAP	(0)
#define ELF_PLATFORM  (NULL)

#define SET_PERSONALITY(ex, ibcs2) do { } while (0)

#endif /* _ASM_MMIX_ELF_H */
