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

#define	SDC_MAX_SECTORS	16
#define	SDC_MAX_PHYS_SEGMENTS	1
#define	SDC_MAX_HW_SEGMENTS	1
#define	VD_IRQ	41

static DEFINE_SPINLOCK(sdc_lock);
static DECLARE_WAIT_QUEUE_HEAD(sdc_wait_int);

struct sdc_reg {
	unsigned char rxtx_buffer[512];
	unsigned char cid[16];
	unsigned char csd[16];
	unsigned int ocr;
	unsigned int sr;
	unsigned short rcs;
	unsigned short pad1;
	unsigned int cmd_arg;
	unsigned short cmd;
	unsigned short pad2;
	unsigned short asr;
	unsigned short pad3;
	unsigned short rr1;
};

volatile struct sdc_reg *sd0_reg = (struct sdc_reg *)0x800100000b000000;

static struct request_queue *sdc_queue;
struct gendisk *sdc_disk;
struct scatterlist sg[SDC_MAX_HW_SEGMENTS];

static unsigned int swap4(unsigned int val)
{
	unsigned int ret;

	ret = (val & 0xff) << 24;
	ret |= (val & 0xff00) << 8;
	ret |= (val >> 8) & 0xff00;
	ret |= (val >> 24) & 0xff;
	return ret;
}

static int sdc_getgeo(struct block_device *bdev, struct hd_geometry *geo)
{
	geo->heads = 64;
	geo->sectors = 63;
	geo->cylinders = 976;
	return 0;
}

static int sdc_ioctl (struct inode *inode,struct file *file,u_int cmd,u_long arg)
{
	switch (cmd) {
	case HDIO_GET_MULTCOUNT:
		return put_user(SDC_MAX_SECTORS, (long __user *) arg);
	default:
		return -EINVAL;
	}
}

static struct block_device_operations sdc_fops = {
	.owner	= THIS_MODULE,
	.ioctl	= sdc_ioctl,
	.getgeo = sdc_getgeo,
};

static struct timer_list sdc_timer;
static void sdc_poll(unsigned long arg)
{
	if (sd0_reg->asr & 0x0400) {
		mod_timer(&sdc_timer, jiffies + 5);
	} else {
		wake_up(&sdc_wait_int);
	}
}

static int sdc_readwrite (u_char operation,volatile struct sdc_reg *sd_reg,
		u_int block,u_int count,
		struct scatterlist *sg, int nsg)
{
	unsigned long save_flags;
	unsigned long addr;

	spin_unlock_irq(&sdc_lock);

	addr = page_to_phys(sg[0].page) + sg[0].offset;

	while (count--) {
		sd_reg->cmd_arg = swap4(block++ << 9);
		if (operation == READ) {
			sd_reg->cmd = 0x1100;
		} else {
			extern void progress(const char * fmt, ...);

			progress("sdc write: not yet!!");
		}
		sdc_timer.expires = jiffies + 10;
		add_timer(&sdc_timer);

		local_irq_save(save_flags);

		sleep_on(&sdc_wait_int);

		local_irq_restore(save_flags);

		del_timer(&sdc_timer);

		if (operation == READ) {
			void *vaddr = phys_to_virt(addr);

			memcpy(vaddr, (void *)sd_reg->rxtx_buffer, 512);
			addr += 512;
		}
	}

	spin_lock_irq(&sdc_lock);
	return (1);
}

static void do_sdc_request (request_queue_t * q)
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
		volatile struct sdc_reg *sd_reg = req->rq_disk->private_data;
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
			printk("do_sdc_request: unknown request\n");
			end_request(req, 0);
			continue;
		}
		nsg = blk_rq_map_sg(q, req, sg);
		res = sdc_readwrite(rw, sd_reg, block, count, sg, nsg);
		if (!end_that_request_first(req, res, count)) {
			blkdev_dequeue_request(req);
			end_that_request_last(req, res);
		}
	}
	busy = 0;
}

static int __init sdc_init(void)
{
	int err;

	if (register_blkdev(HD_MAJOR, "sdc"))
		return -EBUSY;

	err = -ENOMEM;
	sdc_queue = blk_init_queue(do_sdc_request, &sdc_lock);
	if (!sdc_queue)
		goto out1;

	printk("sdcard: Detected drive.");

	err = -ENOMEM;
	sdc_disk = alloc_disk(5);
	if (!sdc_disk)
		goto out2;

	sdc_disk->major = HD_MAJOR;
	sdc_disk->first_minor = 0;
	sprintf(sdc_disk->disk_name, "sdc");
	sdc_disk->fops = &sdc_fops;
	sdc_disk->private_data = (void *)sd0_reg;
	sdc_disk->queue = sdc_queue;
	sdc_disk->capacity = 64 * 63 * 976;
	printk(" %s: %ld block.\n", sdc_disk->disk_name, sdc_disk->capacity);

	init_timer(&sdc_timer);
	sdc_timer.data = 0;
	sdc_timer.function = sdc_poll;

	blk_queue_max_sectors(sdc_queue, SDC_MAX_SECTORS);
	blk_queue_max_phys_segments(sdc_queue, SDC_MAX_PHYS_SEGMENTS);
	blk_queue_max_hw_segments(sdc_queue, SDC_MAX_HW_SEGMENTS);
	add_disk(sdc_disk);

	return 0;

out2:
	blk_cleanup_queue(sdc_queue);
out1:
	unregister_blkdev(HD_MAJOR, "sdc");
	return err;
}

module_init(sdc_init);
