#ifndef _ASM_MMIX_SCATTERLIST_H
#define _ASM_MMIX_SCATTERLIST_H

#ifdef __KERNEL__

struct scatterlist {
	struct page *page;
	unsigned int offset;
	unsigned int length;
};

#endif /* __KERNEL__ */
#endif /* _ASM_MMIX_SCATTERLIST_H */
