/*
 * include/asm-mmix/atomic.h
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

#ifndef _ASM_MMIX_ATOMIC_H_
#define _ASM_MMIX_ATOMIC_H_

typedef struct { volatile long counter; } atomic_t;

#ifdef __KERNEL__

/*
 * primitive.
 */
static __inline__ long __atomic_add_return(int a, atomic_t *v)
{
	long	t;

	__asm__ __volatile__(
	"syncd	7,%2,0\n\
1:	ldo	%0,%2,0\n\
	put	rP,%0\n\
	add	%0,%0,%1\n\
	set	$0,%0\n\
	cswap	$0,%2,0\n\
	bz	$0,1b"
	: "=&r"(t)
	: "r"(a), "r"(&v->counter)
	: "$0", "memory");

	return t;
}

#define ATOMIC_INIT(i)		{ (i) }

#define atomic_read(v)		((int)((v)->counter))
#define atomic_set(v,i)		(((v)->counter) = (i))

static __inline__ void atomic_add(int a, atomic_t *v)
{
	__atomic_add_return(a, v);
}

static __inline__ int atomic_add_return(int a, atomic_t *v)
{
	return (int)__atomic_add_return(a, v);
}

#define atomic_add_negative(a, v)	(atomic_add_return((a), (v)) < 0)

static __inline__ void atomic_sub(int a, atomic_t *v)
{
	__atomic_add_return(-a, v);
}

static __inline__ int atomic_sub_return(int a, atomic_t *v)
{
	return __atomic_add_return(-a, v);
}

static __inline__ void atomic_inc(atomic_t *v)
{
	__atomic_add_return(1, v);
}

static __inline__ int atomic_inc_return(atomic_t *v)
{
	return (int)__atomic_add_return(1, v);
}

#define atomic_inc_and_test(v) (atomic_inc_return(v) == 0)

static __inline__ void atomic_dec(atomic_t *v)
{
	__atomic_add_return(-1, v);
}

static __inline__ int atomic_dec_return(atomic_t *v)
{
	return (int)__atomic_add_return(-1, v);
}

#define atomic_xchg(v, new) (xchg(&((v)->counter), new))

static __inline__ int atomic_add_unless(atomic_t *v, int a, int u)
{
	int t;

	__asm__ __volatile__ (
"1:	ldo	%0,%1,0		# t = v->counter;\n\
	cmp	$0,%0,%3	# $0 = t - u;\n\
	bz	$0,2f		# if (t ==u) end;\n\
	put	rP,%0	\n\
	add	$0,%0,%2	# $0 = t + a;\n\
	cswap	$0,%1,0		# cswap $0 & v->counter;\n\
	bz	$0,1b	\n\
2:"
	:"=&r"(t)
	:"r"(&v->counter),"r"(a),"r"(u)
	:"$0","memory");

	return t != u;
}

#define atomic_inc_not_zero(v) atomic_add_unless((v), 1, 0)

#define atomic_sub_and_test(a, v)	(atomic_sub_return((a), (v)) == 0)
#define atomic_dec_and_test(v)		(atomic_dec_return((v)) == 0)

static __inline__ int atomic_dec_if_positive(atomic_t *v)
{
	long	t;

	__asm__ __volatile__(
	"syncd	7,%1,0\n\
1:	ldo	%0,%1,0\n\
	put	rP,%0\n\
	sub	%0,%0,1\n\
	bn	%0,2f\n\
	set	$0,%0\n\
	cswap	$0,%1,0\n\
	bz	$0,1b\n\
2:"	: "=&r"(t)
	: "r" (&v->counter)
	: "$0", "memory");

	return (int)t;
}

#define smp_mb__before_atomic_dec()     smp_mb()
#define smp_mb__after_atomic_dec()      smp_mb()
#define smp_mb__before_atomic_inc()     smp_mb()
#define smp_mb__after_atomic_inc()      smp_mb()

typedef atomic_t atomic64_t;

#define atomic64_read(v)	((v)->counter)
#define atomic64_set(v,i)	(((v)->counter) = (i))

static __inline__ void atomic64_add(long a, atomic64_t *v)
{
	__atomic_add_return(a, v);
}

static __inline__ void atomic64_sub(long a, atomic64_t *v)
{
	__atomic_add_return(-a, v);
}

static __inline__ void atomic64_inc(atomic64_t *v)
{
	__atomic_add_return(1, v);
}

static __inline__ void atomic64_dec(atomic64_t *v)
{
	__atomic_add_return(-1, v);
}

#include <asm-generic/atomic.h>
#endif /* __KERNEL__ */

#endif	/* _ASM_MMIX_ATOMIC_H_ */
