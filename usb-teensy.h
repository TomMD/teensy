// usb-teensy.h  should only be needed by the host
// (linux kernel) side.

typedef struct {
	int	nr_reads;
	size_t	bytes_read;
	int	nr_writes;
	size_t	bytes_written;
} stats_t;

struct teensy_dev {

};
