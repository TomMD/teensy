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
	USB_Descriptor_Endpoint_t             ControlInEndpoint;
	USB_Descriptor_Endpoint_t             ControlOutEndpoint;
	USB_Descriptor_Endpoint_t             DataInEndpoint;
	USB_Descriptor_Endpoint_t             DataOutEndpoint;
} USB_Descriptor_Configuration_t;

#define CTRL_IN_EPNUM 3
#define CTRL_OUT_EPNUM 4
#define BULK_IN_EPNUM 5
#define BULK_OUT_EPNUM 6

#define BULK_EPSIZE sizeof(uint16_t)
#define CTRL_OUT_EPSIZE 16 // sizeof(usb_cmd_t)
#define CTRL_IN_EPSIZE 16 //sizeof(gadget_err_t)

#endif /* DESC_H */
