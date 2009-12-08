#include <string.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <stdint.h>

#include <LUFA/Drivers/USB/LowLevel/Endpoint.h>
#include <LUFA/Drivers/USB/LowLevel/Device.h>
#include <LUFA/Drivers/USB/USB.h>
#include <LUFA/Drivers/USB/LowLevel/LowLevel.h>
#include <LUFA/Drivers/USB/HighLevel/ConfigDescriptor.h>
#include <LUFA/Drivers/USB/HighLevel/StdDescriptors.h>

#include "../commands.h"

#ifndef DESC_H
#define DESC_H

typedef struct {
	USB_Descriptor_Configuration_Header_t Config;
	USB_Descriptor_Interface_t            Interface;
	USB_Descriptor_Endpoint_t             DataInEndpoint;
	USB_Descriptor_Endpoint_t             DataOutEndpoint;
	USB_Descriptor_Endpoint_t             CommandEndpoint;
} USB_Descriptor_Configuration_t;

#define BULK_IN_EPNUM 1
#define BULK_OUT_EPNUM 2
#define COMMAND_EPNUM 3

#define BULK_OUT_EPSIZE (sizeof(uint16_t))
#define BULK_IN_EPSIZE (sizeof(uint16_t))
#define COMMAND_EPSIZE (sizeof(usb_cmd_t))

#endif /* DESC_H */
