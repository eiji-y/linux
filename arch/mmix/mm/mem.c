#include <linux/kernel.h>
#include <linux/mm.h>
#include <asm/cacheflush.h>

void copy_user_page(void *vto, void *vfrom, unsigned long vaddr,
		    struct page *pg)
{
//	printk("copy_user_page(vto = %p, vfrom = %p, vaddr = %lx, pg = %p).\n", vto, vfrom, vaddr, pg);

	copy_page(vto, vfrom);
	flush_dcache_page(pg);
}
