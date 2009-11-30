// usb-teensy.h  should only be needed by the host
// (linux kernel) side.

#include <linux/fs.h>

ssize_t teensy_read(struct file *filp, char __user *buf, size_t count,
                    loff_t *pos);
ssize_t teensy_write(struct file *filp, const char __user *buf, size_t count,
                     loff_t *pos);
int teensy_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long idx);

typedef struct {
	int	nr_reads;
	size_t	bytes_read;
	int	nr_writes;
	size_t	bytes_written;
} stats_t;

#define NUM_ENDPOINTS 4

struct teensy_dev {
  struct usb_device    *device;        /* device information storage */
  struct usb_interface *interface;     /* interface information storage */
  struct semaphore sem;                /* use device as mutex granularity */
  __u8                 del_endpoint;   /* CONTROL (OUT) for delete requests */
  __u8                 err_endpoint;   /* CONTROL (IN) for error info */
  __u8                 read_endpoint;  /* BULK (IN) for reads */
  size_t               read_size;      /* size of the read endpoint */
  __u8                 write_endpoint; /* BULK (OUT) for writes */
  size_t               write_size;     /* size of the write endpoint */
  struct kref          num_open;       /* reference count for device handles */
  stats_t              teensy_stats;   /* statistics for device */
};
