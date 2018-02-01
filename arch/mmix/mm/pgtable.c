#include <linux/mm.h>

unsigned long ioremap_bot;

void show_mem(void)
{
	extern void progress(const char * fmt, ...);
	progress("show_mem(): Not Yet\n");
}

#define	PGDIR_ORDER	3

static int b[5] = {0, 2, 4, 6, 8};

static inline int pgd_level(unsigned long address)
{
	if (address & (~0L << 53))
		return 4;
	if (address & (~0L << 43))
		return 3;
	if (address & (~0L << 33))
		return 2;
	if (address & (~0L << 23))
		return 1;
	return 0;
}

pgd_t *pgd_offset(struct mm_struct *mm, unsigned long address)
{
	int	seg;

//	printk("pgd_offset() address = %lx\n", address);

	if (address & 0x8000000000000000) {
		panic("pgd_offset(): invalid address %lx", address);
	}
	seg = (address >> 61) & 0x3;
//	printk("b[seg = %d] = %d\n", seg, b[seg]);

	return mm->pgd + b[seg] * 1024;
}

pgd_t *pgd_alloc(struct mm_struct *mm)
{
	pgd_t	*ret;

	ret = (pgd_t*)__get_free_pages(GFP_KERNEL|__GFP_ZERO, PGDIR_ORDER);
	return ret;
}

static void ptp_free(pte_t *ptp, int level)
{
	int	i;

//	printk("ptp_free: ptp = %p, level = %d\n", ptp, level);
	level--;
	for (i = 0; i < 1024; i++, ptp++)
		if (*ptp & 0x8000000000000000) {
//			printk("ptp = %p, *ptp = %lx\n", ptp, *ptp);

			if (level)
				ptp_free((pte_t *)(*ptp & 0xffffffffffffe000), level);
//			printk("free_page = %lx\n", *ptp & 0xffffffffffffe000);
			free_page(*ptp & 0xffffffffffffe000);
		}
}

void pgd_free(struct mm_struct *mm)
{
	int	seg;
	int	level;

	for (seg = 0; seg < 4; seg++) {
		pte_t *ptp = (pte_t *)(mm->pgd + b[seg] * 1024);

		for (level = 1; level < b[seg + 1] - b[seg]; level++) {
			ptp_free(ptp + level * 1024, level);
		}
	}
	free_pages((unsigned long)mm->pgd, PGDIR_ORDER);
}

static inline pte_t *ptp_alloc(pte_t *ptp)
{
	pte_t *pte = (pte_t *)__get_free_page(GFP_KERNEL|__GFP_ZERO);

	if (pte)
		*ptp = (pte_t)((unsigned long)pte | 0x8000000000000000);
	return pte;
}

extern pte_t *pte_offset_map(pmd_t *pmd, unsigned long addr)
{
	int	level;
	int	seg;
	pte_t	*ptp = (pte_t *)pmd;

//	printk("pte_offset_map(): pmd = %p, addr = %lx\n", pmd, addr);
	seg = (addr >> 61) & 0x3;
	level = pgd_level(addr & 0x1fffffffffffffff);
//	printk("pte_offset_map(): seg = %d, level = %d\n", seg, level);
	if (level >= b[seg + 1] - b[seg]) {
		printk("pte_offset_map(): out of segment size\n");
		return NULL;
	}
	ptp += 1024 * level;
//	printk("ptp = %p\n", ptp);
	switch (level) {
	case 3:
		ptp = ptp + ((addr >> 43) & 0x3ff);
//		printk("pte_offset_map(): level 3 ptp = %p\n", ptp);
		if (!(*ptp & 0x8000000000000000) && !ptp_alloc(ptp))
			return NULL;
		ptp = (pte_t *)((*ptp) & 0x8000fffffffffe00);
//		printk("level 2 ptp = %p\n", ptp);
		// fall thru
	case 2:
		ptp = ptp + ((addr >> 33) & 0x3ff);
//		printk("level 2 ptp = %p\n", ptp);
		if (!(*ptp & 0x8000000000000000) && !ptp_alloc(ptp))
			return NULL;
		ptp = (pte_t *)((*ptp) & 0x8000fffffffffe00);
//		printk("level 1 ptp = %p\n", ptp);
		// fall thru
	case 1:
		ptp = ptp + ((addr >> 23) & 0x3ff);
//		printk("level 1 ptp = %p\n", ptp);
		if (!(*ptp & 0x8000000000000000) && !ptp_alloc(ptp))
			return NULL;
		ptp = (pte_t *)((*ptp) & 0x8000fffffffffe00);
//		printk("level 0 pte = %p\n", ptp);
		// fall thru
	case 0:
		ptp = ptp + ((addr >> 13) & 0x3ff);
//		printk("pte = %p\n", ptp);
		break;
	default:
//		printk("pte_offset_map(): level %d is Not Yet\n", level);
		for (;;)
			;
		break;
	}
//	printk("pte_offset_map(): pte = %p\n", ptp);
	return ptp;
}

pte_t *pte_alloc_one_kernel(struct mm_struct *mm, unsigned long address)
{
	extern void progress(const char * fmt, ...);
	progress("pte_alloc_one_kernel(): Not Yet\n");
	return 0;
}

struct page *pte_alloc_one(struct mm_struct *mm, unsigned long address)
{
	extern void progress(const char * fmt, ...);
	progress("pte_alloc_one(): Not Yet\n");
	return 0;
}

void pte_free_kernel(pte_t *pte)
{
	extern void progress(const char * fmt, ...);
	progress("pte_free_kernel(): Not Yet\n");
}
