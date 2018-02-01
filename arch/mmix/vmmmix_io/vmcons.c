#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/console.h>
#include <linux/interrupt.h>

static struct tty_driver *vmcons_driver;
static int open_count = 0;

static irqreturn_t
vmcons_interrupt(int irq, void *dev_id, struct pt_regs *regs)
{
	struct tty_struct *tty = (struct tty_struct *)dev_id;
	int *port = (int *)0x8001000000000004;
	int input = *port;

	if (input & 0x0100) {
		tty_insert_flip_char(tty, input & 0xff, 0);
		tty_flip_buffer_push(tty);
		*port = 0;
	}
	return IRQ_HANDLED;
}

static int vmcons_tty_open(struct tty_struct *tty, struct file *filp)
{
	if (!open_count++)
		request_irq(40, vmcons_interrupt, 0, "vmcons", tty);
	return 0;
}

void vmcons_tty_close(struct tty_struct * tty, struct file * filp)
{
	if (!--open_count)
		free_irq(40, tty);
}

static int vmcons_tty_write(struct tty_struct *tty,
			const unsigned char *buf, int count)
{
	int *port = (int *)0x8001000000000000;
	int i;

	for (i = 0; i < count; i++) {
		while (*port & 0x100)
			;
		*port = *buf++|0x100;
	}
	return count;
}

static int vmcons_tty_write_room(struct tty_struct *tty)
{
	return 0x100000;
}

static int vmcons_tty_chars_in_buffer(struct tty_struct *tty)
{
	return 0;
}

static struct tty_operations vmcons_ops = {
	.open			= vmcons_tty_open,
	.close			= vmcons_tty_close,
	.write			= vmcons_tty_write,
	.write_room		= vmcons_tty_write_room,
	.chars_in_buffer	= vmcons_tty_chars_in_buffer,
};

static int __init vm_cons_init(void)
{
	struct tty_driver *driver;
	int ret;

	driver = alloc_tty_driver(1);
	if (!driver) {
		printk("vmcons: alloc_tty_driver() failed.\n");
		return -ENOMEM;
	}

	driver->driver_name	= "ttyS";
	driver->name		= "vmconsole";
	driver->major		= TTY_MAJOR;
	driver->minor_start	= 64;
	driver->type		= TTY_DRIVER_TYPE_SYSCONS;
	driver->init_termios	= tty_std_termios;
	tty_set_operations(driver, &vmcons_ops);
	ret = tty_register_driver(driver);
	if (ret) {
		put_tty_driver(driver);
		return ret;
	}
	vmcons_driver = driver;
	return 0;
}

module_init(vm_cons_init);

/* console */

static void putch(int c)
{
	int *port = (int *)0x8001000000000000;

	if (c == '\n')
		putch('\r');

	while (*port & 0x100)
		;
	*port = c|0x100;
}

static void vmcons_console_write(struct console *c, const char *s, unsigned n)
{
	while (n-- && *s)
		putch(*s++);
}

static struct tty_driver *
vmcons_console_device(struct console *co, int *index)
{
	*index = co->index;
	return vmcons_driver;
}

static struct console vmcons_info =
{
	.name	= "vmcons",
	.write	= vmcons_console_write,
	.device	= vmcons_console_device,
	.flags	= CON_PRINTBUFFER|CON_BOOT,
	.index	= -1,
};

void __init
register_vm_console(void)
{
	register_console(&vmcons_info);
}
