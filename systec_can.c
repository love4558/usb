#include <linux/init.h>
#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/usb.h>
#include <linux/kref.h>
#include <linux/workqueue.h>
//#define NET_DEVICE

#define to_usb_dev(d)	container_of(d, struct usb_dev_sample, kref)


struct usb_device_id systec_can_main_table[] = {
{USB_DEVICE(0x878, 0x1104)},	// sysWORXX USB-CANmodule1
{}
};

static struct workqueue_struct *cmd_wq;

MODULE_DEVICE_TALBE(usb, systec_can_main_talbe);

static struct usb_driver systec_can_driver;

#ifdef	NET_DEVICE
static const struct net_device_ops systec_netdev_ops;
#else
static struct file_operations systec_netdev_ops;
#endif

struct usb_dev_sample{
	struct usb_device * 	udev;
	struct usb_interface * 	interface;
	unsigned char * 	bulk_in_buffer;
	size_t			bulk_in_size;
	__u8			bulk_in_endpointAddr;
	__u8			bulk_out_endpointAddr;
	struct kref		kref;
};

static struct usb_class_driver usb_class = {
	.name = "usb/sytec_can%d",
	.fops = &systec_netdev_ops,
	.mode = S_IFCHR | S_IRUSR | S_IWUSR | S_IRGRP | S_IWGPR | S_IROTH,
	.minor_base = USB_SKEL_MINOR_BASE,
};

/*********************************************************************
define the function for systec_can_driver struct
********************************************************************/
static void usb_delete(kref *kref);

static void systec_can_main_disconnect(struct usb_interface *intf)
{
	struct usb_dev_sample *dev;
	int minor = interface -> minor;

	dev = usb_get_intfdata(interface);
	usb_set_infdata(interface, NULL);	
	usb_deregister_dev(interface, &usb_class);

	kref_put(&dev->kref, usb_delete);
}

static void systec_can_main_probe(struct usb_interface *intf, const struct usb_device_id *id)
{

}


static struct usb_driver systec_can_driver = {
	.name  = KBUILD_MODNAME,
	.probe = systec_can_main_probe,
	.disconnect = systec_can_main_disconnect,
	.id_table = systec_can_main_table,
};

#ifdef NET_DEVICE
static int systec_can_open(struct net_device *netdev);

#else
static int systec_can_open(struct inode *inode, struct file *file)
{
	struct usb_dev_sample *dev;
	int subminor;
	struct usb_interface *interface;
	int retval = 0;

	subminor = iminor(inode);
	interface = usb_find_interface(&systec_can_driver, subminor);
	if (!interface){
		err("%s - error, cann't find device for minor %d\n", __FUNCTION__, subminor);
		retval = -ENODEV;		
	}

	dev = usb_get_intfdata(interface);
	if(!dev){
		retval = -ENODEV;
		goto  exit;
	}
	
	file -> private_data = dev;
	kref_get(&dev->kref);
exit:
	return retval;
}
#endif

#ifdef NET_DEVICE
static int systec_can_close(struct net_device *netdev);

#else
static void usb_delete(struct kref *kref)
{
	struct usb_dev_sample *dev;
	dev = to_usb_dev(kref);
	usb_put_dev(dev->udev);
	kfree(dev->bulk_in_buffer);
	kfree(dev);
}

static int systec_can_close(struct inode *inode, struct file *file)
{
	struct usb_dev_sample *dev;
	dev = (struct usb_dev_sample *)file-> private_data;

	kref_put(&dev->kref,usb_delete);
	return 0;	
}
#endif


static netdev_tx_t systec_can_start_xmit(struct sk_buff *skb, 
				struct net_device *netdev);

static void systec_can_read_stat_callback(struct urb *urb);

static void systec_can_write_stat_callback(struct urb *urb);


#ifdef NET_DEVICE
static const struct net_device_ops systec_netdev_ops = {
	.ndo_open = systec_can_open,
	.ndo_stop = systec_can_close,
	.ndo_start_xmit = systec_can_start_xmit,
};
#else
static struct file_operations systec_netdev_ops = {
	.owner = THIS_MODULE,
	.open  = systec_can_open,
	.read  = systec_can_read_stat_callback,
	.write = systec_can_write_stat_callback,
	.release = systec_can_close,
};



#endif



static int __init systec_can_main_init(void)
{
	int err;
	pr_debug("systec_can_main_init\n");

	if(!cmd_wq){
		cmd_wq = create_workqueue("systec_wq");
		if(!cmd_wq){
			err("	error:could not create workqueue\n");
			return(-1);
		}
	}
	
	err = usb_register(&systec_can_driver);
	if(err){
		destroy_workqueue(cmd_wq);
		err("usb_register faild. Error number %d\n", err);
		return err;
	}

	return 0;
}

static __exit systec_can_main_exit(void)	
{
	pr_debug("systec_can_main_exit\n");
	destroy_workqueue(cmd_wq);
	usb_deregister(&systec_can_driver);
}

module_init(systec_can_main_init);
module_exit(systec_can_main_exit);

MODULE_AUTHOR("Zhang xiaopeng <zhangxiaopeng0829@gmail.com>");
MODULE_LICENSE("GPL");
