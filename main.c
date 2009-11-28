/*------------------------------------------------------------------------------
 * CS 572 Assignment 2
 * teensy-usb driver
 * Author: Rebekah Leslie
 * Group: Thomas DuBuisson and John Ochsner
 * Additional code sources: Inspiration and code snipits derived from the 
 *                          ldd examples (particularly scullc) by Alessandro 
 *                          Rubini and Jonathan Corbet.
------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/
/* Includes                                                                */
/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/

#include <linux/usb.h>

/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/
/* Function Prototypes                                                     */
/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/

static int teensy_init(void);
static void teensy_exit(void);
int teensy_open(struct inode *inode, struct file *filp);
int teensy_release(struct inode *inode, struct file *filp);
int teensy_probe(struct usb_interface *intf, const struct usb_device_id *id);
void teensy_disconnect(struct usb_interface *intf);

/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/
/* Global Data                                                             */
/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/

#define VENDOR_ID   0x2f2f   /* unique id from linux-usb.org/usb.ids */
#define PRODUCT_ID  0x1001   /* random product number */

/* structure describing the devices supported by this driver */
static struct usb_device_id supported_devs[] = {
  { USB_DEVICE(VENDOR_ID, PRODUCT_ID) }, { }
};

/* turn device-id structure into appropriate format for kernel */
MODULE_DEVICE_TABLE(usb, supported_devs);

/* structure describing the normal file operations for our device */
static struct file_operations teensy_fops = {
  .owner      = THIS_MODULE,
  .open       = teensy_open,
  .release    = teensy_release,
#if 0 // wait until Thomas implements
  .read       = teensy_read,
  .write      = teensy_write,
  .ioctl      = teensy_ioctl
#endif
};

/* structure describing USB-specific driver information */
static struct usb_driver teensy_driver = {
  .name       = "teensy",
  .id_table   = supported_devs,
  .probe      = teensy_probe,
  .disconnect = teensy_disconnect,
};

/* class driver structure so that we can use the USB major number */
static struct usb_class_driver teensy_class_driver = {
  .name       = "usb/teensy%d",
  .fops       = &teensy_fops,
};

/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/
/* Module Information                                                      */
/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/

MODULE_AUTHOR("Rebekah Leslie, Thomas DuBuisson, John Ochsner");
MODULE_LICENSE("Dual BSD/GPL");

/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/
/* Driver Initialization/Cleanup                                           */
/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/
static int teensy_init(void) {
  printk(KERN_ALERT "Initializing teensy store device.\n");
  int result;

  /* register us as a USB driver with USB core */
  result = usb_register(&teensy_driver);
  if (result) { printk("ERROR: usb_register failed.\n"); }

  return result;
}

/* Register initialization routine */
module_init(teensy_init);

static void teensy_exit(void) {
  printk(KERN_ALERT "Exiting teensy store device.\n");

  /* unregister with USB core */
  usb_deregister(&teensy_driver);
}

/* Register exit routine */
module_exit(teensy_exit);

/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/
/*  Driver Open/Close                                                      */
/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/
int teensy_open(struct inode *inode, struct file *filp) {
  printk(KERN_ALERT "Opened teensy store device.\n");
  return 0;
}

int teensy_release(struct inode *inode, struct file *filp) {
  printk(KERN_ALERT "Closing teensy store device.\n");
  return 0;
}

/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/
/*  Driver Probe/Disconnect                                                */
/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/
int teensy_probe(struct usb_interface *intf, const struct usb_device_id *id) {
  printk(KERN_ALERT "Probing for teensy device.\n");
  return 0;
}

void teensy_disconnect(struct usb_interface *intf) {

}

/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/
/* Proc Interface                                                          */
/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/
