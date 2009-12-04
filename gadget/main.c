#include <string.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <stdint.h>

#include <LUFA/Drivers/USB/LowLevel/Endpoint.h>
// #include <LUFA/Drivers/USB/LowLevel/Device.h>
#include <LUFA/Drivers/USB/USB.h>
// #include <LUFA/Drivers/USB/LowLevel/LowLevel.h>
// #include <LUFA/Drivers/USB/HighLevel/ConfigDescriptor.h>
// #include <LUFA/Drivers/USB/HighLevel/StdDescriptors.h>

#include "desc.h"

int read_cmd()
{
//	Endpoint_SelectEndpoint(CTRL_OUT_EPNUM);
//	Endpoint_
	return 0;
}

void EVENT_USB_Device_ConfigurationChanged(void)
{
	Endpoint_ConfigureEndpoint(CTRL_OUT_EPNUM, EP_TYPE_CONTROL, ENDPOINT_DIR_OUT, CTRL_EPSIZE, ENDPOINT_BANK_DOUBLE);
	Endpoint_ConfigureEndpoint(CTRL_IN_EPNUM,  EP_TYPE_CONTROL, ENDPOINT_DIR_IN,  CTRL_EPSIZE, ENDPOINT_BANK_DOUBLE);
	Endpoint_ConfigureEndpoint(BULK_OUT_EPNUM, EP_TYPE_BULK,    ENDPOINT_DIR_OUT, BULK_EPSIZE, ENDPOINT_BANK_DOUBLE);
	Endpoint_ConfigureEndpoint(BULK_IN_EPNUM,  EP_TYPE_BULK,    ENDPOINT_DIR_IN,  BULK_EPSIZE, ENDPOINT_BANK_DOUBLE);
}

void handle_cmd(int cmd)
{
}

int main(void)
{
	int cmd;

	USB_Init();
	while(cmd = read_cmd()) {
		handle_cmd(cmd);
	}
	return 0;
}
