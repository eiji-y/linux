/*
 * arch/mmix/kernel/ptrace.c
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

#include <linux/ptrace.h>

void ptrace_disable(struct task_struct *child)
{
	extern void progress(const char * fmt, ...);
	progress("ptrace_disable(): Not Yet\n");
}

long arch_ptrace(struct task_struct *child, long request, long addr, long data)
{
	extern void progress(const char * fmt, ...);
	progress("arch_ptrace(): Not Yet\n");
	return 0;
}
