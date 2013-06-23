#include <linux/init.h>
#include <linux/module.h>
#include <linux/usb.h>
#include <linux/workqueue.h>


static struct usb_device_id systec_can_main_table[] = {
{USB_DEVICE(0x878, 0x1104)},	// sysWORXX USB-CANmodule1

};

static struct workqueue_struct *cmd_wq;

MODULE_DEVICE_TALBE(usb, systec_can_main_talbe);

static struct usb_driver systec_can_driver;

#ifdef	NET_DEVICE
static const struct net_device_ops systec_netdev_ops;
#else
static struct file_operations systec_netdev_ops;
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
	usb_register(&systec_can_driver);
}

module_init(systec_can_main_init);
module_exit(systec_can_main_exit);

MODULE_AUTHOR("Zhang xiaopeng <zhangxiaopeng0829@gmail.com>");


