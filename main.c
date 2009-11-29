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
#include "usb-teensy.h"

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
void on_teensy_close(struct kref *kref);

/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/
/* Global Data                                                             */
/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/

#define VENDOR_ID   0x16c0   /* id from PJRC-supplied udev rules file */
#define PRODUCT_ID  0x0478   /* TODO: get actual product number with udevinfo */

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
  int result;
  
  printk(KERN_ALERT "teensy: Initializing teensy store device.\n");

  /* register us as a USB driver with USB core */
  result = usb_register(&teensy_driver);
  if (result) { printk("ERROR: usb_register failed.\n"); }

  return result;
}

/* Register initialization routine */
module_init(teensy_init);

static void teensy_exit(void) {
  printk(KERN_ALERT "teensy: Exiting teensy store device.\n");

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
  struct usb_interface *interface; /* storage for the usb interface */
  struct teensy_dev *dev;          /* storage for our device structure */
  int minornum = iminor(inode);    /* minor number for this dev instance */

  printk(KERN_ALERT "Opened teensy store device.\n");

  /* get the usb interface pointer for this driver and device */
  interface = usb_find_interface(&teensy_driver, minornum);
  if (!interface) {
    printk("ERROR: no device interface found\n");
    return -ENODEV;
  }

  /* access the device-local data from the interface (set up by probe) */
  dev = (struct teensy_dev*) usb_get_intfdata(interface);
  if (!dev) { return -ENODEV; }

  /* increment the usage count for the device */
  kref_get(&dev->num_open);

  /* save the devive-local structure for other fops functions */
  filp->private_data = dev;

  return 0;
}

int teensy_release(struct inode *inode, struct file *filp) {
  struct teensy_dev *dev = (struct teensy_dev*) filp->private_data;
  if (!dev) { return -ENODEV; }
  
  printk(KERN_ALERT "Closing teensy store device.\n");

  /* decrement the usage count for the device */
  kref_put(&dev->num_open, &on_teensy_close);

  return 0;
}

void on_teensy_close(struct kref *kref) {
  printk("Closed last open handle to teensy store.\n");

  /* TODO: could do something more interesting here */
}

/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/
/*  Driver Probe/Disconnect                                                */
/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/
int teensy_probe(struct usb_interface *intf, const struct usb_device_id *id) {
  struct teensy_dev *dev;
  int result = 0;

  printk(KERN_ALERT "Probing for teensy device.\n");

  /* allocate memory for the device-local storage  */
  dev = kmalloc(sizeof(struct teensy_dev), GFP_KERNEL);
  if (!dev) { return -ENOMEM; }

  /* initialize the structure */
  dev->device = interface_to_usbdev(intf); /* get and save device structure */
  dev->interface = intf;                   /* save interface structure */
  kref_init(&dev->num_open);               /* initialize open count */
  dev->teensy_stats.nr_reads      = 0;     /* initialize statistics info */
  dev->teensy_stats.bytes_read    = 0;
  dev->teensy_stats.nr_writes     = 0;
  dev->teensy_stats.bytes_written = 0;

  /* increment reference count for device */
  usb_get_dev(dev->device);
  
  /* configure endpoints */
  // TODO

  /* save teensy_dev structure in the interface for later access */
  usb_set_intfdata(dev->interface, dev);

  /* register the device */
  result = usb_register_dev(dev->interface, &teensy_class_driver);
  if (!result) { goto done; /* success */ }

  /* failure branch: free resources */
  usb_set_intfdata(dev->interface, NULL);
  kref_put(&dev->num_open, on_teensy_close);
  usb_put_dev(dev->device);
  kfree(dev);
done:
  return result;
}

void teensy_disconnect(struct usb_interface *intf) {
  struct teensy_dev *dev = usb_get_intfdata(intf);

  /* deregister minor number with usb core */
  usb_deregister_dev(dev->interface, &teensy_class_driver);

  /* clear the pointer in the interface structure */
  usb_set_intfdata(dev->interface, NULL);

  /* decrement reference count for device */
  usb_put_dev(dev->device);

  /* decrement reference count for user handles to device */
  kref_put(&dev->num_open, on_teensy_close);

  /* free the device-local storage data */
  kfree(dev);
}

/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/
/* Proc Interface                                                          */
/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/

