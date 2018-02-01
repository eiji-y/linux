#include <linux/mm.h>
#include <linux/string.h>

#include <asm/uaccess.h>

unsigned long
copy_to_user(void __user *to, const void *from, unsigned long n)
{
	BUG_ON((long)n < 0);
	if (access_ok(VERIFY_WIRTE, to , n))
		n = __copy_to_user(to, from, n);
	return n;
}

unsigned long
copy_from_user(void *to, const void __user *from, unsigned long n)
{
	if (likely(access_ok(VERIFY_READ, from, n)))
		n = __copy_from_user(to, from, n);
	else
		memset(to, 0, n);
	return n;
}

#define	__do_clear_user(addr, size)				\
do {								\
	__asm__ __volatile__(					\
		"	bz	%0,2f\n"			\
		"	setl	$0,0\n"				\
		"1:	stbu	$0,%1,0\n"			\
		"	addu	%1,%1,1\n"			\
		"	subu	%0,%0,1\n"			\
		"	pbnz	%0,1b\n"			\
		"2:\n"						\
		".section __ex_table,\"a\"\n"			\
		"	.align 8\n"				\
		"	octa	1b,2b\n"			\
		".previous\n"					\
		:"+r"(size)					\
		:"r"(addr)					\
		:"memory","$0");				\
} while (0)

unsigned long clear_user(void __user *addr, unsigned long size)
{
//	printk("clear_user(addr = %p, size = %ld):\n", addr, size);

	might_sleep();
	if (access_ok(VERIFY_WRITE, addr, size))
		__do_clear_user(addr, size);
//	printk("clear_user(): return = %ld\n", size);
	return size;
}

extern unsigned long __copy_tofrom_user(void __user *to,
		const void __user *from, unsigned long size)
{
//	printk("__copy_tofrom_user(): to = %p, from = %p, size = %ld\n", to, from, size);
	__asm__ __volatile__(
		"	bz	%0,3f\n"
		"1:	ldbu	$3,%2,0\n"
		"2:	stbu	$3,%1,0\n"
		"	addu	%2,%2,1\n"
		"	addu	%1,%1,1\n"
		"	subu	%0,%0,1\n"
		"	pbnz	%0,1b\n"
		"3:\n"
		".section __ex_table,\"a\"\n"
		"	.align 8\n"
		"	octa	1b,3b\n"
		"	octa	2b,3b\n"
		".previous\n"
		:"+r"(size)
		:"r"(to),"r"(from)
		:"memory","$3");
	return size;
}

int __strncpy_from_user(char *dst, const char __user *src, long count)
{
	long res;

	__asm__ __volatile__(
		"	setl	%0,0\n"
		"	bz	%3,2f\n"
		"1:	ldb	$4,%2,%0\n"
		"	stbu	$4,%1,%0\n"
		"	bz	$4,2f\n"
		"	addu	%0,%0,1\n"
		"	cmp	$4,%0,%3\n"
		"	pbnz	$4,1b\n"
		"2:\n"
		".section __ex_table,\"a\"\n"
		"	.align 8\n"
		"	octa	1b,2b\n"
		".previous\n"
		: "=&r"(res)
		: "r"(dst), "r"(src),"r"(count)
		: "$4","memory");

	return res;
}

int strnlen_user(const char __user *str, long len)
{
	long	res;

	might_sleep();
	__asm__ __volatile__(
		"	set	%0,0\n"
		"	bz	%2,2f\n"
		"1:	ldb	$3,%1,%0\n"
		"	addu	%0,%0,1\n"
		"	bz	$3,2f\n"
		"	cmp	$3,%0,%2\n"
		"	pbnz	$3,1b\n"
		"2:\n"
		".section __ex_table,\"a\"\n"
		"	.align 8\n"
		"	octa	1b,2b\n"
		".previous\n"
		: "=&r"(res)
		: "r"(str),"r"(len)
		: "$3");

	return res;
}
