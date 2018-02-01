#ifndef _MMIX_BITOPS_H
#define _MMIX_BITOPS_H

#include <linux/compiler.h>
#include <asm/hw_irq.h>

#define smp_mb__before_clear_bit()	smp_mb()
#define smp_mb__after_clear_bit()	smp_mb()

#include <asm-generic/bitops.h>

#endif /* _MMIX_BITOPS_H */
