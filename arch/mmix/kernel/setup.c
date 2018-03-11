/*
 * arch/mmix/kernel/setup.c
 *
 *   Copyright (C) 2008-2009 Eiji Yoshiya (eiji-y@pb3.so-net.ne.jp)
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

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/console.h>
#include <linux/seq_file.h>
#include <linux/bootmem.h>
#include <linux/mm.h>
#include <linux/thread_info.h>
#include <linux/root_dev.h>

#include <asm/page.h>

unsigned long boot_mem_size;

struct thread_info *xxx(atomic_t *counter)
{
	struct thread_info *ti;

	ti = current_thread_info();
	return ti;
}

void machine_restart(char *cmd)
{
	extern void progress(const char * fmt, ...);
	progress("machine_restart(): Not Yet\n");
}

void machine_power_off(void)
{
	extern void progress(const char * fmt, ...);
	progress("machine_power_off(): Not Yet\n");
}

void machine_halt(void)
{
	extern void progress(const char * fmt, ...);
	progress("machine_halt(): Not Yet\n");
}

void (*pm_power_off)(void) = machine_power_off;

void progress(const char * fmt, ...)
{
	int *port = (int *)0x8001000000000000;
	char ch;

	while ((ch = *fmt++)) {
		while (*port & 0x100)
			;
		*port = ch|0x100;
	}

	dump_stack();
	for (;;)
		;
}

int show_cpuinfo(struct seq_file *m, void *v)
{
	extern void progress(const char * fmt, ...);
	progress("show_cpuinfo(): Not Yet\n");
	return 0;
}

static void *c_start(struct seq_file *m, loff_t *pos)
{
	extern void progress(const char * fmt, ...);
	progress("c_start(): Not Yet\n");
	return 0;
}

static void *c_next(struct seq_file *m, void *v, loff_t *pos)
{
	extern void progress(const char * fmt, ...);
	progress("c_next(): Not Yet\n");
	return 0;
}

static void c_stop(struct seq_file *m, void *v)
{
	extern void progress(const char * fmt, ...);
	progress("c_stop(): Not Yet\n");
}

struct seq_operations cpuinfo_op = {
	.start =c_start,
	.next =	c_next,
	.stop =	c_stop,
	.show =	show_cpuinfo,
};

void __init setup_arch(char **cmdline_p)
{
	// TODO:
#ifdef	CONFIG_VMMMIX
	extern void register_vm_console(void);
#endif
	static char cmdline[512];
#ifdef	CONFIG_VMMMIX
	char *src = (char *)0x8000000001000000;
	char *dst = cmdline;

	while (*src)
		*dst++ = *src++;
#endif
	*cmdline_p = cmdline;

#ifdef	CONFIG_VMMMIX
	boot_mem_size = 0x10000000;
	register_vm_console();
#endif
#ifdef	CONFIG_DE0
	boot_mem_size = 0x800000;
#endif
{
	unsigned long bootmap_size;
	extern char _end[];
	unsigned long start;

	start = __pa(PAGE_ALIGN((unsigned long)_end));
	/*
	 * Initialize the boot-time allocator (with low memory only):
	 */
	bootmap_size = init_bootmem(start >> PAGE_SHIFT,
					boot_mem_size >> PAGE_SHIFT);
	free_bootmem(start + bootmap_size,
			boot_mem_size - (start + bootmap_size));
}
{
	unsigned long zones_size[MAX_NR_ZONES], i;

	/*
	 * All pages are DMA-able so we put them all in the DMA zone.
	 */
	zones_size[ZONE_DMA] = boot_mem_size >> PAGE_SHIFT;
	for (i = 1; i < MAX_NR_ZONES; i++)
		zones_size[i] = 0;

	free_area_init(zones_size);
}

	printk("setup_arch(): Not Yet\n");
}
