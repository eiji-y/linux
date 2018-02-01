/*
 * include/asm-mmix/uaccess.h
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

#ifndef _ARCH_MMIX_UACCESS_H
#define _ARCH_MMIX_UACCESS_H

#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/thread_info.h>
#include <linux/prefetch.h>
#include <linux/string.h>
#include <linux/sched.h>

#include <asm/current.h>

#define VERIFY_READ 0
#define VERIFY_WRITE 1

#define MAKE_MM_SEG(s)  ((mm_segment_t) { (s) })
#define KERNEL_DS	MAKE_MM_SEG(~0UL)
#define USER_DS		MAKE_MM_SEG(TASK_SIZE - 1)

#define get_ds()	(KERNEL_DS)
#define get_fs()	(current->thread.fs)
#define set_fs(val)	(current->thread.fs = (val))

#define segment_eq(a,b)      ((a).seg == (b).seg)

#define	__access_ok(addr, size, segment)	\
	(((addr) <= (segment).seg) && (((addr) + (size) - 1) <= (segment).seg))

#define access_ok(type, addr, size)		\
	(__chk_user_ptr(addr),			\
	 __access_ok((__force unsigned long)(addr), (size), get_fs()))

#define get_user(x, ptr) \
	__get_user_check((x), (ptr), sizeof(*(ptr)))

#define put_user(x,ptr)						\
	__put_user_check((__typeof__(*(ptr)))(x), (ptr), sizeof(*(ptr)))

#define __get_user(x, ptr) \
	__get_user_nocheck((x), (ptr), sizeof(*(ptr)))

#define __put_user(x, ptr) \
	__put_user_nocheck((__typeof__(*(ptr)))(x), (ptr), sizeof(*(ptr)))

extern long __put_user_bad(void);

#define	__put_user_asm(x, addr, err, op)			\
	__asm__ __volatile__(					\
		"1:	" op "	%1,%2,0\n"			\
		"2:\n"						\
		".section .fixup,\"ax\"\n"			\
		"3:	negu	%0,0,%3\n"			\
		"	jmp	2b\n"				\
		".previous\n"					\
		".section __ex_table,\"a\"\n"			\
		"	.align 8\n"				\
		"	octa	1b,3b\n"			\
		".previous"					\
		: "+r" (err)					\
		: "r" (x), "r" (addr), "i"(EFAULT))

#define	__put_user_size(x, ptr, size, retval)			\
do {								\
	retval = 0;						\
	switch (size) {						\
	case 1: __put_user_asm(x, ptr, retval, "stbu"); break;	\
	case 2: __put_user_asm(x, ptr, retval, "stwu"); break;	\
	case 4: __put_user_asm(x, ptr, retval, "sttu"); break;	\
	case 8: __put_user_asm(x, ptr, retval, "stou"); break;	\
	default: __put_user_bad();				\
	}							\
} while (0)

#define	__put_user_nocheck(x, ptr, size)			\
({ 								\
	long __pu_err;						\
	__typeof__(*(ptr)) __user *__pu_addr = (ptr);		\
	if (!is_kernel_addr((unsigned long)__pu_addr))		\
		might_sleep();					\
	__chk_user_ptr(ptr);					\
	__put_user_size((x), __pu_addr, (size), __pu_err);	\
	__pu_err;						\
})

#define __put_user_check(x, ptr, size)                                  \
({                                                                      \
        long __pu_err = -EFAULT;                                        \
        __typeof__(*(ptr)) __user *__pu_addr = (ptr);                   \
        might_sleep();                                                  \
        if (access_ok(VERIFY_WRITE, __pu_addr, size))                   \
                __put_user_size((x), __pu_addr, (size), __pu_err);      \
        __pu_err;                                                       \
})

/* get */
extern long __get_user_bad(void);

#define	__get_user_asm(x, addr, err, op)			\
	__asm__ __volatile__(					\
		"1:	"op" %1,%2,0	# get_user\n"		\
		"2:\n"						\
		".section .fixup,\"ax\"\n"			\
		"3:	negu	%0,0,%3\n"			\
		"	jmp	2b\n"				\
		".previous\n"					\
		".section __ex_table,\"a\"\n"			\
		"	.align 8\n"				\
		"	octa	1b,3b\n"			\
		".previous\n"					\
		: "+r" (err), "=r"(x)				\
		: "r" (addr), "i"(EFAULT))

#define	__get_user_size(x, ptr, size, retval)			\
do {								\
	retval = 0;						\
	__chk_user_ptr(ptr);					\
	switch (size) {						\
	case 1: __get_user_asm(x, ptr, retval, "ldbu"); break;	\
	case 2: __get_user_asm(x, ptr, retval, "ldwu"); break;	\
	case 4: __get_user_asm(x, ptr, retval, "ldtu"); break;	\
	case 8: __get_user_asm(x, ptr, retval, "ldou"); break;	\
	default: (x) = __get_user_bad();			\
	}							\
} while (0)

#define	__get_user_nocheck(x, ptr, size) 			\
({								\
	long __gu_err;						\
	unsigned long __gu_val;					\
	const __typeof(*(ptr)) __user *__gu_addr = (ptr);	\
	__chk_user_ptr(ptr);					\
	if (!is_kernel_addr((unsigned long)__gu_addr))		\
		might_sleep();					\
	__get_user_size(__gu_val, __gu_addr, (size), __gu_err);	\
	(x) = (__typeof__(*(ptr)))__gu_val;			\
	__gu_err;						\
})

#define	__get_user_check(x, ptr, size)					\
({									\
	long __gu_err = -EFAULT;					\
	unsigned long __gu_val = 0;					\
	const __typeof(*(ptr)) __user *__gu_addr = (ptr);		\
	might_sleep();							\
	if (access_ok(VERIFY_READ, __gu_addr, (size)))			\
		__get_user_size(__gu_val, __gu_addr, (size), __gu_err);	\
	(x) = (__typeof__(*(ptr)))__gu_val;				\
	__gu_err;							\
})

extern unsigned long __copy_tofrom_user(void __user *to,
		const void __user *from, unsigned long size);

extern unsigned long copy_to_user(void __user *to,
				const void *from, unsigned long n);
extern unsigned long copy_from_user(void *to, const void __user *from,
				    unsigned long n);

static inline unsigned long __copy_to_user_inatomic(void __user *to,
		const void *from, unsigned long n)
{
	if (__builtin_constant_p(n) && (n <= 8)) {
		unsigned long ret;

		switch (n) {
		case 1:
			__put_user_size(*(u8 *)from, (u8 __user *)to, 1, ret);
			break;
		case 2:
			__put_user_size(*(u16 *)from, (u16 __user *)to, 2, ret);
			break;
		case 4:
			__put_user_size(*(u32 *)from, (u32 __user *)to, 4, ret);
			break;
		case 8:
			__put_user_size(*(u64 *)from, (u64 __user *)to, 8, ret);
			break;
		}
		if (ret == 0)
			return 0;
	}
	return __copy_tofrom_user(to, (__force const void __user *)from, n);
}

static inline unsigned long __copy_to_user(void __user *to,
		const void *from, unsigned long size)
{
	might_sleep();
	return __copy_to_user_inatomic(to, from, size);
}

static inline unsigned long __copy_from_user_inatomic(void *to,
		const void __user *from, unsigned long n)
{
	if (__builtin_constant_p(n) && (n <= 8)) {
		unsigned long ret;

		switch (n) {
		case 1:
			__get_user_size(*(u8 *)to, from, 1, ret);
			break;
		case 2:
			__get_user_size(*(u8 *)to, from, 2, ret);
			break;
		case 4:
			__get_user_size(*(u8 *)to, from, 4, ret);
			break;
		case 8:
			__get_user_size(*(u8 *)to, from, 8, ret);
			break;
		}
		if (ret == 0)
			return 0;
	}
	return __copy_tofrom_user((__force void __user *)to, from, n);
}

static inline unsigned long __copy_from_user(void *to,
		const void __user *from, unsigned long size)
{
	might_sleep();
	return __copy_from_user_inatomic(to, from, size);
}

extern int __strncpy_from_user(char *dst, const char __user *src, long count);

static inline long strncpy_from_user(char *dst, const char __user *src,
		long count)
{
	might_sleep();
	if (likely(access_ok(VERIFY_READ, src, 1)))
		return __strncpy_from_user(dst, src,count);
	return EFAULT;
}

extern int strnlen_user(const char __user *str, long len);

struct exception_table_entry {
	unsigned long insn;
	unsigned long fixup;
};

unsigned long __must_check clear_user(void __user *addr, unsigned long size);

#include <asm-generic/uaccess.h>

#endif	/* _ARCH_MMIX_UACCESS_H */
