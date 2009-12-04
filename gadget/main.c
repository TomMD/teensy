#include <string.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <stdint.h>

// #include <LUFA/Drivers/USB/LowLevel/Endpoint.h>
// #include <LUFA/Drivers/USB/LowLevel/Device.h>
#include <LUFA/Drivers/USB/USB.h>
// #include <LUFA/Drivers/USB/LowLevel/LowLevel.h>
// #include <LUFA/Drivers/USB/HighLevel/ConfigDescriptor.h>
// #include <LUFA/Drivers/USB/HighLevel/StdDescriptors.h>

// #include "desc.h"

int main(void)
{
	USB_Init();
	return 0;
}
