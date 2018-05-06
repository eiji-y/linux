#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/console.h>
#include <linux/interrupt.h>

// DE0 VGA Character Buffer
#define	VRAM	0x800100000a000000
#define	SCR_WIDTH 80
#define	SCR_HEIGHT 60

static int cx = 0;
static int cy = 0;
static char *vram = (char *)VRAM;

static void scroll(void)
{
	int i, j;
	char *dst, *src;

	for (i = 0; i < SCR_HEIGHT - 1; i++) {

		dst = (char *)VRAM + (i << 7);
		src = dst + (1 << 7);

		for (j = 0; j < SCR_WIDTH; j++)
			*dst++ = *src++;
	}

	dst = (char *)VRAM + (i << 7);
	for (j = 0; j < SCR_WIDTH; j++)
		*dst++ = 0;
}

void de0_putch(char c)
{
	switch (c) {
	default:
		*vram++ = c;
		if (++cx == SCR_WIDTH) {
			cx = 0;
			if (cy == SCR_HEIGHT - 1) {
				scroll();
			} else {
				cy++;
			}
			vram = (char *)VRAM + (cy << 7);
		}
		break;
	case '\r':
		vram -= cx;
		cx = 0;
		break;
	case '\n':
		if (cy == SCR_HEIGHT - 1) {
			scroll();
		} else {
			cy++;
			// vram += SCR_WIDTH;
			vram += (1 << 7);
		}
		break;
	case '\b':
		if (cx > 0) {
			cx--;
			vram--;
		}
		break;
	}
}

static struct tty_driver *de0_console_driver;
#if	0
static int open_count = 0;

static irqreturn_t
de0_cons_interrupt(int irq, void *dev_id, struct pt_regs *regs)
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
#endif

static int de0_cons_tty_open(struct tty_struct *tty, struct file *filp)
{
#if	0
	if (!open_count++)
		request_irq(40, de0_cons_interrupt, 0, "de0_cons", tty);
#endif
	return 0;
}

void de0_cons_tty_close(struct tty_struct * tty, struct file * filp)
{
#if	0
	if (!--open_count)
		free_irq(40, tty);
#endif
}

static int de0_cons_tty_write(struct tty_struct *tty,
			const unsigned char *buf, int count)
{
	int i;

	for (i = 0; i < count; i++) {
		de0_putch(*buf++);
	}
	return count;
}

static int de0_cons_tty_write_room(struct tty_struct *tty)
{
	return 0x100000;
}

static int de0_cons_tty_chars_in_buffer(struct tty_struct *tty)
{
	return 0;
}

static struct tty_operations de0_cons_ops = {
	.open			= de0_cons_tty_open,
	.close			= de0_cons_tty_close,
	.write			= de0_cons_tty_write,
	.write_room		= de0_cons_tty_write_room,
	.chars_in_buffer	= de0_cons_tty_chars_in_buffer,
};

static int __init de0_console_init(void)
{
	struct tty_driver *driver;
	int ret;

	driver = alloc_tty_driver(1);
	if (!driver) {
		printk("de0_cons: alloc_tty_driver() failed.\n");
		return -ENOMEM;
	}

	driver->driver_name	= "ttyS";
	driver->name		= "de0console";
	driver->major		= TTY_MAJOR;
	driver->minor_start	= 64;
	driver->type		= TTY_DRIVER_TYPE_SYSCONS;
	driver->init_termios	= tty_std_termios;
	tty_set_operations(driver, &de0_cons_ops);
	ret = tty_register_driver(driver);
	if (ret) {
		put_tty_driver(driver);
		return ret;
	}
	de0_console_driver = driver;
	return 0;
}

module_init(de0_console_init);

/* console */

static void de0_console_write(struct console *c, const char *s, unsigned n)
{
	while (n-- && *s) {
		char ch;

		ch = *s++;
		if (ch == '\n')
			de0_putch('\r');
		de0_putch(ch);
	}
}

static struct tty_driver *
de0_console_device(struct console *co, int *index)
{
	*index = co->index;
	return de0_console_driver;
}

static struct console de0_console_info =
{
	.name	= "de0_cons",
	.write	= de0_console_write,
	.device	= de0_console_device,
	.flags	= CON_PRINTBUFFER|CON_BOOT,
	.index	= -1,
};

void __init
register_de0_console(void)
{
	register_console(&de0_console_info);
}

#if	0
static void prt_hex_one(unsigned char val)
{
	val &= 0xf;
	if (val < 10)
		de0_putch('0' + val);
	else
		de0_putch('a' + val - 10);
}

void de0_prt_hex(unsigned long val)
{
	prt_hex_one(val >> 60);
	prt_hex_one(val >> 56);
	prt_hex_one(val >> 52);
	prt_hex_one(val >> 48);
	prt_hex_one(val >> 44);
	prt_hex_one(val >> 40);
	prt_hex_one(val >> 36);
	prt_hex_one(val >> 32);
	prt_hex_one(val >> 28);
	prt_hex_one(val >> 24);
	prt_hex_one(val >> 20);
	prt_hex_one(val >> 16);
	prt_hex_one(val >> 12);
	prt_hex_one(val >> 8);
	prt_hex_one(val >> 4);
	prt_hex_one(val >> 0);
	de0_putch('\r');
	de0_putch('\n');
}
#endif
