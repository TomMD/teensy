/* user-ops.c for the teensy-usb driver
 * Author: Thomas M. DuBuisson
 * License: GPL
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

#include "teensy-usb.h"

extern stats_t teensy_stats;

// START /dev/teensy interface
// 	(read, write, ioctl, open, release)
teensy_read(struct_file *filp, char __user *buf, size_t count, loff_t *pos)
{
	struct command cmd;
	int err=0;

	err = copy_from_user(&cmd, buf, sizeof(struct command));
	if (err) {
		printk(KERN_WARNING "teensy-usb: read: Could not copy from user: %d\n", err);
		goto teensy_read_out;
	}

	if(dat.cmd_type != CMD_READ) {
		printk(KERN_WARNING "teensy-usb: read: Invalid command %d\n", dat.cmd_type);
		err = -EINVAL;
		goto teensy_read_out;
	}

	// FIXME issue usb_read(cmd)
	
teensy_read_out:
	return err;
}

teensy_write(struct file *filp, const char __user *buf, size_t count, loff_t *pos)
{
	struct command cmd;
	int err=0;

	err = copy_from_user(&cmd, buf, sizeof(struct command));
	if (err) {
		printk(KERN_WARNING "teensy-usb: write: Could not copy from user: %d\n", err);
		goto teensy_write_out;
	}

	if (dat.cmd_type != CMD_STORE) {
		printk(KERN_WARNING "teensy-usb: write: invalid command: %d\n", dat.cmd_type);
		err = -EINVAL;
		goto teensy_write_out;
	}

	// FIXME perform usb_write(cmd);

teensy_write_out:
	return err;
}

teensy_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long idx)
{
	int res = 0;

	if (_IOC_TYPE(cmd) != TEENSY_MAGIC)
		return -ENOTTY;
	if (_IOC_NR(cmd) >= TEENY_IOC_MAXNR)
		return -ENOTTY;

	switch (cmd) {
		case TEENY_IOCFLUSH:
			// FIXME flush all data on the teensy
			break;
		case TEENSY_IOCERASE_IDX:
			// FIXME erase given index (the 'idx' argument)
			break;
		default:
			res = -EINVAL;
	}
	return res;
}

// START /proc/teensy-stats interface
int teensy_read_stats(char *buf, char **start, off_t offset, int count,
		int *eof, void *data)
{
	return snprintf(buf, PAGE_SIZE,
		"Teensy statistics\n"
		"\t# reads: %d\n"
		"\t# writes: $d\n"
		"\t# bytes read: %d\n"
		"\t# bytes written: %d\n"
		"\t# current stores: %d\n"
		"\t# btes stored: %d\n",
		teensy_stats.nr_reads,
		teensy_stats.nr_writes,
		teensy_stats.bytes_read,
		teensy_stats.bytes_written);
}
