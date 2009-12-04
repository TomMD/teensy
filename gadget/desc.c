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

#include "desc.h"

uint16_t CALLBACK_USB_GetDescriptor
	(const uint16_t	wValue,
	 const uint8_t	wIndex,
	 void **const	DescriptorAddress,
	 uint8_t *	MemoryAddressSpace)
{
        const uint8_t  DescriptorType   = (wValue >> 8);
        const uint8_t  DescriptorNumber = (wValue & 0xFF);

        void*    Address = NULL;
        uint16_t Size    = NO_DESCRIPTOR;

	switch (DescriptorType) {
                case DTYPE_Device:
                        Address = (void*)&DeviceDescriptor;
                        Size    = sizeof(USB_Descriptor_Device_t);
                        break;
                case DTYPE_Configuration:
                        Address = (void*)&ConfigurationDescriptor;
                        Size    = sizeof(USB_Descriptor_Configuration_t);
                        break;
                case DTYPE_String:
                        switch (DescriptorNumber)
                        {
                                case 0x00:
                                        Address = (void*)&LanguageString;
                                        Size    = pgm_read_byte(&LanguageString.Header.Size);
                                        break;
                                case 0x01:
                                        Address = (void*)&ManufacturerString;
                                        Size    = pgm_read_byte(&ManufacturerString.Header.Size);
                                        break;
                                case 0x02:
                                        Address = (void*)&ProductString;
                                        Size    = pgm_read_byte(&ProductString.Header.Size);
                                        break;
                        }

                        break;
        }

        *DescriptorAddress = Address;
        return Size;
}
