// commands.h
// Author: Thomas M. DuBuisson
// Defines commands for teensy/PC interaction
// also abused to hold the IOCTL numbers

#ifndef COMMANDS_H
#define COMMANDS_H

#ifndef __KERNEL__
#include <stdint.h>
#endif

// Commands that travel over the COMMAND usb channel
#define CMD_STORE	0x00
#define CMD_READ	0x03
#define CMD_DELETE	0x06
#define CMD_NULL	0xFF

// This is the KERNEL/USB Driver to GADGET command buffer structure
struct usb_cmd {
	uint8_t cmd_type;
	uint64_t index;
} __attribute__((__packed__));

typedef struct usb_cmd usb_cmd_t;

struct gadget_err {
	uint8_t err;
} __attribute__((__packed__));

typedef struct gadget_err gadget_err_t;

// This is the USER SPACE to KERNEL command structures
struct command {
	uint8_t cmd_type;
	uint64_t index;
	uint16_t data;
	// For now lets just do the above.  In the future:
	// uint16_t len; // Command line data ends here
	// void *data;	 // Travels over BULK OUT line
} __attribute__((__packed__));

typedef struct command command_t;

#define MAXDATASIZE sizeof(uint16_t)

// Responses that travel over the Error usb line
#define ENOERR 0
#define EFULL 1
#define EINVALIDINDEX 2
#define EINVALIDCOMMAND 3
#define EWTF 4

struct response {
	uint8_t result;
} __attribute__((__packed__));

#define TEENSY_MAGIC 't'
#define TEENSY_IOCFLUSH _IO(TEENSY_MAGIC,1)
#define TEENSY_IOCERASE_IDX _IO(TEENSY_MAGIC,2)
#define TEENSY_IOC_MAXNR 3

#endif
