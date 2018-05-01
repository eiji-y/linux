/*
 * include/asm-mmix/ptrace.h
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

#ifndef _ASM_MMIX_PTRACE_H
#define _ASM_MMIX_PTRACE_H

struct pt_regs {
	unsigned long	r0;
	unsigned long	r1;
	unsigned long	r253;
	unsigned long	r255;
	unsigned long	rJ;
	unsigned long	rBB;
	unsigned long	rWW;
	unsigned long	rXX;
	unsigned long	rYY;
	unsigned long	rZZ;
	unsigned long	orig_rBB;
};

#define profile_pc(regs) \
({ \
	extern void progress(const char * fmt, ...); \
	progress("profile_pc(): Not Yet\n"); \
	(long)0; \
})

#define user_mode(regs) (((regs)->rWW & 0x8000000000000000) == 0)

#endif /* _ASM_MMIX_PTRACE_H */
