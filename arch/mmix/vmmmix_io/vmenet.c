#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/string.h>
#include <linux/ptrace.h>
#include <linux/errno.h>
#include <linux/ioport.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/spinlock.h>
#include <linux/mii.h>
#include <linux/workqueue.h>
#include <linux/bitops.h>

#define TX_TIMEOUT	(2*HZ)

#define	VM_ENET_DEV_ADDR	0x8001000000010000

#define	VM_ENET_RX_RING_SIZE	16
#define	VM_ENET_TX_RING_SIZE	16

#define	VM_ENET_STATUS_EXISTS	0x0000000000000001
#define	VM_ENET_COMMAND_START	0x0000000000000001

#define	VM_ENET_BUF_BUSY	0x8000000000000000
#define	VM_ENET_BUF_LEN_MASK	0x7fff000000000000
#define	VM_ENET_BUF_ADDR_MASK	0x0000ffffffffffff

#define	RX_BUF_SIZE		(1500 + 14)
#define	NUM_BUFS_PER_PAGE	(PAGE_SIZE / RX_BUF_SIZE)
#define	NUM_BUFS_PAGES		((VM_ENET_RX_RING_SIZE + NUM_BUFS_PER_PAGE - 1) / NUM_BUFS_PER_PAGE)

struct vm_enet_dev {
	unsigned long	status;
	unsigned long	command;
	unsigned char	mac_addr[6];
	unsigned char	pad[2];
	unsigned long	rx_buf[VM_ENET_RX_RING_SIZE];
	unsigned long	tx_buf[VM_ENET_TX_RING_SIZE];
};

struct vm_enet_private {
	struct	sk_buff* tx_skbuff[VM_ENET_TX_RING_SIZE];

	int cur_rx, cur_tx;
	int dirty_tx;
	volatile struct vm_enet_dev *ep;

	unsigned long	mem_addr[NUM_BUFS_PAGES];

	struct	net_device_stats stats;
	uint	tx_free;
	spinlock_t lock;
};

static irqreturn_t
vm_enet_interrupt(int irq, void * dev_id, struct pt_regs * regs)
{
	struct	net_device *dev = dev_id;
	struct	vm_enet_private *vep = (struct vm_enet_private *)dev->priv;
	volatile struct	vm_enet_dev	*ved = vep->ep;

	while (vep->tx_free != VM_ENET_TX_RING_SIZE) {
		if (ved->tx_buf[vep->dirty_tx] & VM_ENET_BUF_BUSY)
			break;

		vep->stats.tx_packets++;

		dev_kfree_skb_irq(vep->tx_skbuff[vep->dirty_tx]);
		vep->tx_skbuff[vep->dirty_tx] = NULL;
		if (vep->dirty_tx == VM_ENET_TX_RING_SIZE - 1)
			vep->dirty_tx = 0;
		else
			vep->dirty_tx++;

		if (!vep->tx_free++) {
			if (netif_queue_stopped(dev)) {
				netif_wake_queue(dev);
			}
		}
	}

	while (!(ved->rx_buf[vep->cur_rx] & VM_ENET_BUF_BUSY)) {
		unsigned long desc = ved->rx_buf[vep->cur_rx];
		struct	sk_buff *skb;
		ushort	pkt_len;

		vep->stats.rx_packets++;

		pkt_len = (desc >> 48) & 0x7fff;
#if	0
printk(" packet recevied. len = %d, va = %p\n", pkt_len, __va(desc & VM_ENET_BUF_ADDR_MASK));
{
	unsigned char *cp = (unsigned char *)__va(desc & VM_ENET_BUF_ADDR_MASK);
	int	i;

	for (i = 0; i < pkt_len; i++)
		printk("%02x", cp[i]);
	printk("\n");
}
#endif
		vep->stats.rx_bytes += pkt_len;

		/* This does 16 byte alignment, much more than we need. */
		skb = dev_alloc_skb(pkt_len + NET_IP_ALIGN);

		if (skb == NULL) {
			printk("%s: Memory squeeze, dropping packet.\n", dev->name);
			vep->stats.rx_dropped++;
		} else {
			skb_reserve(skb, NET_IP_ALIGN);
			skb->dev = dev;
			skb_put(skb,pkt_len);	/* Make room */
			eth_copy_and_sum(skb,
				(unsigned char *)__va(desc & VM_ENET_BUF_ADDR_MASK),
				pkt_len, 0);
			skb->protocol=eth_type_trans(skb,dev);
			netif_rx(skb);
		}

		ved->rx_buf[vep->cur_rx] |= VM_ENET_BUF_BUSY;
		if (vep->cur_rx == VM_ENET_TX_RING_SIZE - 1)
			vep->cur_rx = 0;
		else
			vep->cur_rx++;
	}

	// ack
	ved->status = ved->status;

	return IRQ_HANDLED;
}

static int
vm_enet_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct vm_enet_private *vep = (struct vm_enet_private *)dev->priv;
	volatile struct	vm_enet_dev	*ved = vep->ep;
	int txp;

	txp = vep->cur_tx;

#ifndef final_version
	if (ved->tx_buf[txp] & VM_ENET_BUF_BUSY) {
		printk("%s: tx queue full!.\n", dev->name);
		return 1;
	}
#endif

	ved->tx_buf[txp] = ((unsigned long)skb->len << 48) | __pa(skb->data);

	spin_lock_irq(&vep->lock);

	/* Save skb pointer. */
	vep->tx_skbuff[txp] = skb;

	vep->stats.tx_bytes += skb->len;

	/* Send */
	ved->tx_buf[txp] |= VM_ENET_BUF_BUSY;

	dev->trans_start = jiffies;

	/* If this was the last BD in the ring, start at the beginning again. */
	if (txp == VM_ENET_TX_RING_SIZE - 1)
		txp = 0;
	else
		txp++;

	if (!--vep->tx_free) {
		netif_stop_queue(dev);
	}

	vep->cur_tx = txp;

	spin_unlock_irq(&vep->lock);

	return 0;
}

static void
vm_enet_timeout(struct net_device *dev)
{
	printk("vm_enet_timeout\n");
}

static int
vm_enet_open(struct net_device *dev)
{
	struct vm_enet_private *vep = dev->priv;
	volatile struct	vm_enet_dev	*ved = vep->ep;
	int	i, j, k;

	if (request_irq(8, vm_enet_interrupt, 0, "vmenet", dev) < 0)
		printk("Can't get VMENET IRQ %d\n", 8);

	vep->cur_rx = 0;
	vep->cur_tx = 0;
	vep->dirty_tx = 0;
	vep->tx_free = VM_ENET_TX_RING_SIZE;

	k = 0;
	for (i = 0; i < NUM_BUFS_PAGES; i++) {
		unsigned long mem_addr;
	
		vep->mem_addr[i] = __get_free_page(GFP_KERNEL);
		mem_addr = __pa(vep->mem_addr[i]);

		for (j = 0; j < NUM_BUFS_PER_PAGE; j++) {
			ved->rx_buf[k++] = VM_ENET_BUF_BUSY|mem_addr;
			mem_addr += RX_BUF_SIZE;
			if (k >= VM_ENET_RX_RING_SIZE)
				goto out;
		}
	}
out:
	for (i = 0; i < VM_ENET_TX_RING_SIZE; i++)
		ved->tx_buf[i] = 0;

	ved->command = VM_ENET_COMMAND_START;

	return 0;
}

static int
vm_enet_close(struct net_device *dev)
{
	printk("vm_enet_close\n");
	return 0;
}

static struct net_device_stats *vm_enet_get_stats(struct net_device *dev)
{
	struct vm_enet_private *vep = (struct vm_enet_private *)dev->priv;

	return &vep->stats;
}

static int __init vm_enet_init(void)
{
	struct net_device *dev;
	struct vm_enet_private *vep;
	volatile struct	vm_enet_dev	*ved;
	int	i, err;

	ved = (struct vm_enet_dev *)VM_ENET_DEV_ADDR;
	if (!(ved->status & VM_ENET_STATUS_EXISTS))
		return -ENODEV;

	/* Create an Ethernet device instance.
	*/
	dev = alloc_etherdev(sizeof(*vep));
	if (!dev)
		return -ENOMEM;


	ved->command = 0;
	for (i = 0; i < 6; i++)
		dev->dev_addr[i] = ved->mac_addr[i];

	vep = dev->priv;
	spin_lock_init(&vep->lock);
	vep->ep = ved;

	dev->base_addr = (unsigned long)(vep->ep);
	dev->open = vm_enet_open;
	dev->hard_start_xmit = vm_enet_start_xmit;
	dev->tx_timeout = vm_enet_timeout;
	dev->watchdog_timeo = TX_TIMEOUT;
	dev->stop = vm_enet_close;
	dev->get_stats = vm_enet_get_stats;
	/*dev->set_multicast_list = vm_set_multicast_list;*/
	/*dev->set_mac_address = vm_enet_set_mac_address;*/

	err = register_netdev(dev);
	if (err) {
		free_netdev(dev);
		return err;
	}

	printk("%s: VM ENET, ", dev->name);
	for (i=0; i<5; i++)
		printk("%02x:", dev->dev_addr[i]);
	printk("%02x\n", dev->dev_addr[5]);

	return 0;
}
module_init(vm_enet_init);
