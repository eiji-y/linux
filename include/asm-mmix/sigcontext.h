#ifndef _ASM_MMIX_SIGCONTEXT_H
#define _ASM_MMIX_SIGCONTEXT_H

/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 */
#include <linux/compiler.h>
#include <asm/ptrace.h>

struct sigcontext {
	unsigned long	_unused[4];
	int		signal;
	int		_pad0;
	unsigned long	handler;
	unsigned long	oldmask;
	struct pt_regs	__user *regs;
};

#endif /* _ASM_MMIX_SIGCONTEXT_H */
