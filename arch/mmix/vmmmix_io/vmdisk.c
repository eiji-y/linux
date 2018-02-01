/*
 * arch/mmix/vmmix_io/vmdisk.c
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

#include <linux/module.h>
#include <linux/errno.h>
#include <linux/interrupt.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/timer.h>
#include <linux/genhd.h>
#include <linux/hdreg.h>
#include <linux/ioport.h>
#include <linux/init.h>
#include <linux/wait.h>
#include <linux/blkdev.h>
#include <linux/blkpg.h>
#include <linux/delay.h>

#include <asm/system.h>
#include <asm/uaccess.h>
#include <asm/page.h>
#include <asm/scatterlist.h>

#define	VD_MAX_SECTORS	32
#define	VD_MAX_PHYS_SEGMENTS	16
#define	VD_MAX_HW_SEGMENTS	16
#define	VD_IRQ	41

static DEFINE_SPINLOCK(vd_lock);
static DECLARE_WAIT_QUEUE_HEAD(vd_wait_int);

struct vm_hdd_reg {
	unsigned int status;
	unsigned int command;
	unsigned long capacity;
	unsigned long blk_addr;
	unsigned long blk_count;
	struct {
		unsigned long addr;
		unsigned long count;
	} dma[VD_MAX_HW_SEGMENTS];
};

volatile struct vm_hdd_reg *vd_reg = (struct vm_hdd_reg *)0x8001000000001000;

static struct request_queue *vd_queue;
struct gendisk *vd_disk;
struct scatterlist sg[VD_MAX_HW_SEGMENTS];

static int vd_getgeo(struct block_device *bdev, struct hd_geometry *geo)
{
	geo->heads = 255;
	geo->sectors = 63;
	geo->cylinders = bdev->bd_disk->capacity / (255 * 63);
	return 0;
}

static int vd_ioctl (struct inode *inode,struct file *file,u_int cmd,u_long arg)
{
	switch (cmd) {
	case HDIO_GET_MULTCOUNT:
		return put_user(VD_MAX_SECTORS, (long __user *) arg);
	default:
		return -EINVAL;
	}
}

static struct block_device_operations vd_fops = {
	.owner	= THIS_MODULE,
	.ioctl	= vd_ioctl,
	.getgeo = vd_getgeo,
};

static irqreturn_t vd_interrupt_handler(int irq, void *dev_id,
					struct pt_regs *regs)
{
	if (vd_reg->status & 1) {
		vd_reg->status &= ~1;
		wake_up(&vd_wait_int);
		return IRQ_HANDLED;
	}
	return IRQ_NONE;
}

static int vd_readwrite (u_char operation,volatile struct vm_hdd_reg *hd_reg,
		u_int block,u_int count,
		struct scatterlist *sg, int nsg)
{
	unsigned long save_flags;
	int	i;

	spin_unlock_irq(&vd_lock);

	hd_reg->blk_addr = block;
	hd_reg->blk_count = count;
	for (i = 0; i < nsg; i++) {
		hd_reg->dma[i].addr = page_to_phys(sg[i].page) + sg[i].offset;
		hd_reg->dma[i].count = sg[i].length;
	}

	local_irq_save(save_flags);

	hd_reg->command = (operation == READ)?1:2;
	sleep_on(&vd_wait_int);

	local_irq_restore(save_flags);

	spin_lock_irq(&vd_lock);
	return (1);
}

static void do_vd_request (request_queue_t * q)
{
	struct request *req;
	static int busy = 0;

	if (busy)
		return;
	busy = 1;

	while ((req = elv_next_request(q)) != NULL) {
		unsigned block = req->sector;
		unsigned count = req->hard_nr_sectors;
		int rw = rq_data_dir(req);
		volatile struct vm_hdd_reg *vd_reg = req->rq_disk->private_data;
		int res = 0;
		int nsg;

		if (!(req->flags & REQ_CMD)) {
			end_request(req, 0);
			continue;
		}
		if (block + count > get_capacity(req->rq_disk)) {
			end_request(req, 0);
			continue;
		}
		if (rw != READ && rw != WRITE) {
			printk("do_vd_request: unknown request\n");
			end_request(req, 0);
			continue;
		}
		nsg = blk_rq_map_sg(q, req, sg);
		res = vd_readwrite(rw, vd_reg, block, count, sg, nsg);
		if (!end_that_request_first(req, res, count)) {
			blkdev_dequeue_request(req);
			end_that_request_last(req, res);
		}
	}
	busy = 0;
}

static int __init vd_init(void)
{
	int err;

	if (vd_reg->capacity == 0)
		return -ENODEV;

	if (register_blkdev(HD_MAJOR, "vd"))
		return -EBUSY;

	err = -ENOMEM;
	vd_queue = blk_init_queue(do_vd_request, &vd_lock);
	if (!vd_queue)
		goto out1;

	printk("vmhd: Detected drive.");

	err = -ENOMEM;
	vd_disk = alloc_disk(16);
	if (!vd_disk)
		goto out2;

	vd_disk->major = HD_MAJOR;
	vd_disk->first_minor = 0;
	sprintf(vd_disk->disk_name, "hda");
	vd_disk->fops = &vd_fops;
	vd_disk->private_data = (void *)vd_reg;
	vd_disk->queue = vd_queue;
	vd_disk->capacity = vd_reg->capacity;
	printk(" %s: %ld block.\n", vd_disk->disk_name, vd_disk->capacity);

	err = -EBUSY;
	if (request_irq(VD_IRQ,vd_interrupt_handler, 0, "vd hard disk", NULL)) {
		printk("vd: unable to get IRQ%d\n",VD_IRQ);
		goto out3;
	}

	blk_queue_max_sectors(vd_queue, VD_MAX_SECTORS);
	blk_queue_max_phys_segments(vd_queue, VD_MAX_PHYS_SEGMENTS);
	blk_queue_max_hw_segments(vd_queue, VD_MAX_HW_SEGMENTS);
	add_disk(vd_disk);

	return 0;

out3:
	put_disk(vd_disk);
out2:
	blk_cleanup_queue(vd_queue);
out1:
	unregister_blkdev(HD_MAJOR, "vd");
	return err;
}

module_init(vd_init);
