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

#include <linux/errno.h>
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

#define VENDOR_ID      0x16c0   /* id from PJRC-supplied udev rules file */
#define PRODUCT_ID     0x0478   /* chosen product id (matches udev rule) */
#define IFACE_CLASS    0xff     /* "vendor specific" class code */
#define IFACE_SUBCLASS 0xaa     /* chosen subclass */
#define PROTOCOL       0        /* ignore protocol */

/* structure describing the devices supported by this driver */
static struct usb_device_id supported_devs[] = {
  { USB_DEVICE_AND_INTERFACE_INFO(VENDOR_ID, PRODUCT_ID,
                                  IFACE_CLASS, IFACE_SUBCLASS, PROTOCOL) }, { }
};

/* turn device-id structure into appropriate format for kernel */
MODULE_DEVICE_TABLE(usb, supported_devs);

/* structure describing the normal file operations for our device */
static struct file_operations teensy_fops = {
  .owner      = THIS_MODULE,
  .open       = teensy_open,
  .release    = teensy_release,
  .read       = teensy_read,
  .write      = teensy_write,
  .ioctl      = teensy_ioctl
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
  int result = 0;

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

  /* take semaphore before reaquiring the dev pointer "for real" */
  /* we do this to obtain mutual exclusion with the disconnect method */
  /* because that might free the dev structure after we've obtained our */
  /* pointer (which we must do to access the semaphore variable. */
  /* though odd, this strategy avoids taking the BKL */
  if (down_interruptible(&(dev->sem))) { return -ERESTARTSYS; }

  /* use the pointer we get now, if NULL, disconnect intervened */
  dev = (struct teensy_dev*) usb_get_intfdata(interface);
  if (!dev) { result = -ENODEV; goto done; }

  /* increment the usage count for the device */
  kref_get(&dev->num_open);

  /* save the devive-local structure for other fops functions */
  filp->private_data = dev;

done:
  up(&(dev->sem));
  return result;
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
  /* E.g., follow sstore example and clear store on last close */
}

/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/
/*  Driver Probe/Disconnect                                                */
/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/
int teensy_probe(struct usb_interface *intf, const struct usb_device_id *id) {
  struct teensy_dev *dev;
  struct usb_host_interface *host;
  struct usb_endpoint_descriptor *curr_endpoint;
  int result = 0;
  int i;

  printk(KERN_ALERT "Probing for teensy device.\n");

  /* allocate memory for the device-local storage  */
  dev = kmalloc(sizeof(struct teensy_dev), GFP_KERNEL);
  if (!dev) { return -ENOMEM; }

  /* initialize the device-local information */
  memset(dev, 0, sizeof(struct teensy_dev)); /* clear fields */
  dev->device = interface_to_usbdev(intf);   /* save usb device structure */
  dev->interface = intf;                     /* save usb interface structure */
  kref_init(&dev->num_open);                 /* initialize open count */
  sema_init(&dev->sem, 1);                   /* initialize semaphore */

  /* increment reference count for device */
  usb_get_dev(dev->device);
  
  /* get host interface (currently in-use version of altsetting) */
  host = dev->interface->cur_altsetting;

  /* fail if the host interface doesn't match our configuration */
  if (host->desc.bNumEndpoints != NUM_ENDPOINTS) { 
    result = -ENOSYS;
    goto fail_after_setup; 
  }

  /* otherwise, we could match, so try to setup endpoints */
  /* search endpoint descriptors in interface looking for expected endpoints */
  for(i = 0; i < host->desc.bNumEndpoints; i++) {
    curr_endpoint = &host->endpoint[i].desc;

    /* setup read_endpoint if this is the bulk in endpoint */
    if (!dev->read_endpoint &&                       /* haven't set EP yet */
        usb_endpoint_is_bulk_in(curr_endpoint)) {    /* type is BULK IN */
      dev->read_endpoint = curr_endpoint->bEndpointAddress;
      dev->read_size     = curr_endpoint->wMaxPacketSize;
    }

    /* setup write_endpoint if this is the bulk in endpoint */
    else if (!dev->write_endpoint &&                 /* haven't set EP yet */
        usb_endpoint_is_bulk_out(curr_endpoint)) {   /* type is BULK OUT */
      dev->write_endpoint = curr_endpoint->bEndpointAddress;
      dev->write_size = curr_endpoint->wMaxPacketSize;
    }

    else if(!dev->cmd_endpoint &&
	usb_endpoint_is_bulk_out(curr_endpoint)) {
      dev->cmd_endpoint = curr_endpoint->bEndpointAddress;
      dev->cmd_size = curr_endpoint->wMaxPacketSize;
    }

    else {
      printk(KERN_WARNING "teensy_usb: unknown endpoint.\n");
      printk(KERN_WARNING "\tin?   %d", usb_endpoint_dir_in(curr_endpoint));
      printk(KERN_WARNING "\tout?  %d", usb_endpoint_dir_out(curr_endpoint));
      printk(KERN_WARNING "\tbulk? %d", usb_endpoint_xfer_bulk(curr_endpoint));
      printk(KERN_WARNING "\tctrl? %d", usb_endpoint_xfer_control(curr_endpoint));
      printk(KERN_WARNING "\tint?  %d", usb_endpoint_xfer_int(curr_endpoint));
      printk(KERN_WARNING "\tisoc? %d", usb_endpoint_xfer_isoc(curr_endpoint));
    }
  }

  if (!dev->read_endpoint || !dev->write_endpoint || !dev->cmd_endpoint) {
    printk(KERN_WARNING "usb-teensy: Bad endpoint (read, write, del, err):"
		"%d %d %d\n", dev->read_endpoint, dev->write_endpoint,
		dev->cmd_endpoint);
    result = -ENOSYS;
    goto fail_after_setup;
  }

  /* save teensy_dev structure in the interface for later access */
  usb_set_intfdata(dev->interface, dev);

  /* register the device */
  result = usb_register_dev(dev->interface, &teensy_class_driver);
  if (!result) { printk(KERN_WARNING "Probed done.  Sizes (RWC): %d %d %d\n", dev->read_size, dev->write_size, dev->cmd_size); goto done; /* success */ }

  /* failure branch: free resources */
  usb_set_intfdata(dev->interface, NULL);
fail_after_setup:
  usb_put_dev(dev->device);
  kfree(dev);
done:
  return result;
}

void teensy_disconnect(struct usb_interface *intf) {
  struct teensy_dev *dev = usb_get_intfdata(intf);

  /* take the lock for mutual exclusion with open method */
  if (down_interruptible(&(dev->sem))) { return; }

  /* deregister minor number with usb core */
  usb_deregister_dev(dev->interface, &teensy_class_driver);

  /* clear the pointer in the interface structure */
  usb_set_intfdata(dev->interface, NULL);

  /* release semaphore */
  up(&(dev->sem));

  /* decrement reference count for device */
  usb_put_dev(dev->device);

  /* free the device-local storage data */
  kfree(dev);
}
