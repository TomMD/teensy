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

#ifndef DESC_H

uint16_t CALLBACK_USB_GetDescriptor
        (const uint16_t wValue,
         const uint8_t  wIndex,
         void **const   DescriptorAddress,
         uint8_t *      MemoryAddressSpace);

/* START Descriptors -- ENTIRELY RIPPED OFF FROM LUFA LIBRARY EXAMPLES!!! */

USB_Descriptor_Device_t PROGMEM DeviceDescriptor =
{
        .Header                 = {.Size = sizeof(USB_Descriptor_Device_t), .Type = DTYPE_Device},

        .USBSpecification       = VERSION_BCD(01.10),
        .Class                  = 0x00,
        .SubClass               = 0x00,
        .Protocol               = 0x00,

        .Endpoint0Size          = FIXED_CONTROL_ENDPOINT_SIZE,

        .VendorID               = 0x03EB,
        .ProductID              = 0x2045,
        .ReleaseNumber          = 0x0000,

        .ManufacturerStrIndex   = 0x01,
        .ProductStrIndex        = 0x02,
        .SerialNumStrIndex      = USE_INTERNAL_SERIAL,

        .NumberOfConfigurations = FIXED_NUM_CONFIGURATIONS
};

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

#define BULK_EPSIZE 64
#define CTRL_EPSIZE 16

USB_Descriptor_Configuration_t PROGMEM ConfigurationDescriptor =
{
        .Config =
                {
                        .Header                 = {.Size = sizeof(USB_Descriptor_Configuration_Header_t), .Type = DTYPE_Configuration},

                        .TotalConfigurationSize = sizeof(USB_Descriptor_Configuration_t),
                        .TotalInterfaces        = 1,

                        .ConfigurationNumber    = 1,
                        .ConfigurationStrIndex  = NO_DESCRIPTOR,

                        .ConfigAttributes       = USB_CONFIG_ATTR_BUSPOWERED,

                        .MaxPowerConsumption    = USB_CONFIG_POWER_MA(100)
                },

        .Interface =
                {
                        .Header                 = {.Size = sizeof(USB_Descriptor_Interface_t), .Type = DTYPE_Interface},

                        .InterfaceNumber        = 0,
                        .AlternateSetting       = 0,

                        .TotalEndpoints         = 4,

                        .Class                  = 0x08,
                        .SubClass               = 0x06,
                        .Protocol               = 0x50,

                        .InterfaceStrIndex      = NO_DESCRIPTOR
                },

	.ControlInEndpoint = {
			.Header			= {.Size = sizeof(USB_Descriptor_Endpoint_t), .Type = DTYPE_Endpoint},
			.EndpointAddress	= (ENDPOINT_DESCRIPTOR_DIR_IN | CTRL_IN_EPNUM),
			.Attributes		= (EP_TYPE_CONTROL | ENDPOINT_ATTR_NO_SYNC),
			.EndpointSize		= CTRL_EPSIZE,
			.PollingIntervalMS	= 0x00
		},

	.ControlOutEndpoint = {
			.Header			= {.Size = sizeof(USB_Descriptor_Endpoint_t), .Type = DTYPE_Endpoint},
			.EndpointAddress	= (ENDPOINT_DESCRIPTOR_DIR_OUT | CTRL_OUT_EPNUM),
			.Attributes		= (EP_TYPE_CONTROL | ENDPOINT_ATTR_NO_SYNC),
			.EndpointSize		= CTRL_EPSIZE,
			.PollingIntervalMS	= 0x00
		},

        .DataInEndpoint =
                {
                        .Header                 = {.Size = sizeof(USB_Descriptor_Endpoint_t), .Type = DTYPE_Endpoint},

                        .EndpointAddress        = (ENDPOINT_DESCRIPTOR_DIR_IN | BULK_IN_EPNUM),
                        .Attributes             = (EP_TYPE_BULK | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA),
                        .EndpointSize           = BULK_EPSIZE,
                        .PollingIntervalMS      = 0x00
                },

        .DataOutEndpoint =
                {
                        .Header                 = {.Size = sizeof(USB_Descriptor_Endpoint_t), .Type = DTYPE_Endpoint},

                        .EndpointAddress        = (ENDPOINT_DESCRIPTOR_DIR_OUT | BULK_OUT_EPNUM),
                        .Attributes             = (EP_TYPE_BULK | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA),
                        .EndpointSize           = BULK_EPSIZE,
                        .PollingIntervalMS      = 0x00
                }

};

USB_Descriptor_String_t PROGMEM LanguageString =
{
        .Header                 = {.Size = USB_STRING_LEN(1), .Type = DTYPE_String},

        .UnicodeString          = {LANGUAGE_ID_ENG}
};

USB_Descriptor_String_t PROGMEM ManufacturerString =
{
        .Header                 = {.Size = USB_STRING_LEN(3), .Type = DTYPE_String},

        .UnicodeString          = L"JRT"
};

USB_Descriptor_String_t PROGMEM ProductString =
{
        .Header                 = {.Size = USB_STRING_LEN(13), .Type = DTYPE_String},
        .UnicodeString          = L"Micro Storage"
};

#endif /* DESC_H */
