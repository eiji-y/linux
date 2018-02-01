#ifndef _ASM_MMIX_HARDIRQ_H
#define _ASM_MMIX_HARDIRQ_H
#ifdef __KERNEL__

#include <asm/irq.h>

typedef struct {
	unsigned int __softirq_pending;	/* set_bit is used on this */
} irq_cpustat_t;

#include <linux/irq_cpustat.h>	/* Standard mappings for irq_cpustat_t above */

#endif /* __KERNEL__ */
#endif /* _ASM_MMIX_HARDIRQ_H */
