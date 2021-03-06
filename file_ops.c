/* user-ops.c for the teensy-usb driver
 * License: Dual BSD/GPL
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/usb.h>

#include "usb-teensy.h"
#include "commands.h"

extern stats_t teensy_stats;

// START /dev/teensy interface
// 	(read, write, ioctl, open, release)
ssize_t teensy_read(struct file *filp, char __user *buf, size_t count,
		    loff_t *pos)
{
	int actualLen=0;
	struct command cmd;
	usb_cmd_t usb_cmd;
	struct teensy_dev *teensy;
	int err=0;

	teensy = filp->private_data;

	if(down_interruptible(&teensy->sem)) {
		printk(KERN_WARNING "teensy-usb: read: down interrupted\n");
		goto teensy_read_out_no_sem;
	}

	err = copy_from_user(&cmd, buf, sizeof(struct command));
	if (err) {
		printk(KERN_WARNING "teensy-usb: read:"
			" Could not copy from user: %d\n", err);
		goto teensy_read_out;
	}

	if(cmd.cmd_type != CMD_READ) {
		printk(KERN_WARNING "teensy-usb: read: Invalid command %d\n",
			cmd.cmd_type);
		goto teensy_read_out;
	}

	usb_cmd.cmd_type = CMD_READ;
	usb_cmd.index = cmd.index;

	err = usb_bulk_msg(teensy->device, usb_sndbulkpipe(teensy->device, teensy->cmd_endpoint), &usb_cmd,
			   sizeof(usb_cmd_t), &actualLen, HZ*10);
	if (err) {
		printk(KERN_WARNING "teensy-usb: read:"
			" Error writing command CMD_READ: %d\n", err);
		goto teensy_read_out;
	}

	err = usb_bulk_msg(teensy->device, usb_rcvbulkpipe(teensy->device, teensy->read_endpoint), (void *)&cmd.data,
			   sizeof(cmd.data), &actualLen, HZ*10);
	if (err || actualLen != sizeof(cmd.data)) {
		printk(KERN_WARNING "teensy-usb: read:"
			" Error on bulk msg: %d\n", err);
		goto teensy_read_out;
	}

	err = copy_to_user(buf, (void *)&cmd, sizeof(struct command));
	if (err) {
		printk(KERN_WARNING "teensy-usb: read:"
			" Error copying to user buffer: %d\n", err);
		actualLen=0;
		goto teensy_read_out;
	}
	
teensy_read_out:
	up(&teensy->sem);
teensy_read_out_no_sem:
	return (ssize_t)actualLen;
}

ssize_t teensy_write(struct file *filp, const char __user *buf, size_t count,
		     loff_t *pos)
{
	struct teensy_dev *teensy;
	struct command cmd;
	usb_cmd_t usb_cmd;
	int err=0, actualLen=0;

	teensy = filp->private_data;

	if (down_interruptible(&teensy->sem)) {
		printk(KERN_WARNING "teensy-usb: write: down interrupted\n");
		goto teensy_write_out_no_sem;
	}

	err = copy_from_user(&cmd, buf, sizeof(struct command));
	if (err) {
		printk(KERN_WARNING "teensy-usb: write:"
			" Could not copy from user: %d\n", err);
		goto teensy_write_out;
	}

	if (cmd.cmd_type != CMD_STORE) {
		printk(KERN_WARNING "teensy-usb: write:"
			" invalid command: %d\n", cmd.cmd_type);
		goto teensy_write_out;
	}

	usb_cmd.cmd_type = CMD_STORE;
	usb_cmd.index = cmd.index;

	err = usb_bulk_msg(teensy->device, usb_sndbulkpipe(teensy->device, teensy->cmd_endpoint), &usb_cmd,
			   sizeof(usb_cmd_t), &actualLen, HZ*10);
	if (err) {
		printk(KERN_WARNING "teensy-usb: write:"
			" Error on cmd msg: %d\n", err);
		actualLen=0;
		goto teensy_write_out;
	}

	err = usb_bulk_msg(teensy->device, usb_sndbulkpipe(teensy->device, teensy->write_endpoint), (void *)&cmd.data,
			   sizeof(cmd.data), &actualLen, HZ*10);
	if (err) {
		printk(KERN_WARNING "teensy-usb: write:"
			" Error on bulk msg: %d\n", err);
		actualLen=0;
		goto teensy_write_out;
	}

teensy_write_out:
	up(&teensy->sem);
teensy_write_out_no_sem:
	return (ssize_t)actualLen;
}

int teensy_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long idx)
{
	struct teensy_dev *teensy;
	int res = 0, actualLen, err;
	uint8_t rand;
	usb_cmd_t usb_cmd;

	if (_IOC_TYPE(cmd) != TEENSY_MAGIC)
		return -ENOTTY;
	if (_IOC_NR(cmd) >= TEENSY_IOC_MAXNR)
		return -ENOTTY;

	teensy = filp->private_data;
	if (down_interruptible(&teensy->sem)) {
		printk(KERN_WARNING "teensy-usb: write: down interrupted\n");
		goto teensy_ioctl_out_no_sem;
	}

	switch (cmd) {
		case TEENSY_IOCFLUSH:
			// FIXME flush all data on the teensy
			break;
		case TEENSY_IOCERASE_IDX:
			usb_cmd.cmd_type = CMD_DELETE;
			usb_cmd.index = (uint64_t)idx;
			err = usb_bulk_msg(teensy->device, usb_sndbulkpipe(teensy->device, teensy->cmd_endpoint), (void *)&usb_cmd,
					sizeof(usb_cmd_t), &actualLen, HZ*10);
			if (err || actualLen != sizeof(usb_cmd_t)) {
				printk(KERN_WARNING "teensy-usb: ioctl: failed sending CMD_DELETE\n");
				goto teensy_ioctl_out;		
			}
			break;
		default:
			res = -EINVAL;
	}

teensy_ioctl_out:
	up(&teensy->sem);
teensy_ioctl_out_no_sem:
	return res;
}
