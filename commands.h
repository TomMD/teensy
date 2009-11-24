// commands.h
// Author: Thomas M. DuBuisson
// Defines commands for teensy/PC interaction


// Commands that travel over the COMMAND usb channel
#define CMD_STORE 0x00
#define CMD_READ  0x03
#define CMD_DELETE 0x06

struct command {
	uint8_t cmd_type;
	uint64_t index;
	uint16_t data;
	// For now lets just do the above.  In the future:
	// uint16_t len; // Command line data ends here
	// void *data;	 // Travels over BULK OUT line
} __attribute__((__packed__));

typedef struct command command_t;


// Responses that travel over the Error usb line
#define ENOERR 0
#define EFULL 1
#define EINVALIDINDEX 2
#define EINVALIDCOMMAND 3
#define EWTF 4

struct response {
	uint8_t result;
} __attribute__((__packed__));
