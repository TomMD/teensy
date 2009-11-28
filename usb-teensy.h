// usb-teensy.h  should only be needed by the host
// (linux kernel) side.

typedef struct {
	int	nr_reads;
	size_t	bytes_read;
	int	nr_writes;
	size_t	bytes_written;
} stats_t;

struct teensy_dev {
  struct usb_device    *device;      /* device information storage */
  struct usb_interface *interface;   /* interface information storage */
  struct kref          num_open;     /* reference count for device handles */  
  stats_t              teensy_stats; /* statistics for device */
};
