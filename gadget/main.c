#include <string.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <avr/power.h>
#include <util/delay.h>
#include <stdint.h>

#include <LUFA/Drivers/USB/LowLevel/Endpoint.h>
// #include <LUFA/Drivers/USB/LowLevel/Device.h>
#include <LUFA/Drivers/USB/USB.h>
// #include <LUFA/Drivers/USB/LowLevel/LowLevel.h>
// #include <LUFA/Drivers/USB/HighLevel/ConfigDescriptor.h>
// #include <LUFA/Drivers/USB/HighLevel/StdDescriptors.h>
#include <LUFA/Drivers/USB/HighLevel/StdRequestType.h>

#include "teensy_display.h"
#include "teensy_blobs.h"
#include "desc.h"
#include "../commands.h"

struct state {
	int index;
	int offset;
	int command;
	uint8_t err;
};

typedef struct state state_t;

void StoreTask(state_t *state);
void Init(void);

int main(void)
{
	state_t state;

	state.index = 0;
	state.offset = 0;
	state.command = CMD_NULL;
	state.err = ENOERR;

	Init();

	while(1) {
		StoreTask(&state);
	}
	return 0;
}

void Init(void)
{
        MCUSR &= ~(1 << WDRF);
        wdt_disable();

	clock_prescale_set(clock_div_1);

	USB_Init();
	Display_Init();
}

#if 1  // Change to zero for shorter StoreTask used for testing

void StoreTask(state_t *state)
{
	uint8_t err;
	uint16_t data;

	if (USB_DeviceState != DEVICE_STATE_Configured)
		return;

	Endpoint_SelectEndpoint(BULK_OUT_EPNUM);
	if (Endpoint_IsOUTReceived() && Endpoint_IsReadWriteAllowed()) {
		if (CMD_STORE == state->command) {
			err = Endpoint_Read_Stream_LE(&data, sizeof(uint16_t));
			// teensy_store(state->index, data);			FIXME
		} else {
			Endpoint_ClearOUT();
		}

	}

	Endpoint_SelectEndpoint(BULK_IN_EPNUM);
	if (Endpoint_IsINReady() && Endpoint_IsReadWriteAllowed()) {
		if(CMD_READ == state->command) {
			// teensy_read(state->index, &data);			FIXME
			err = Endpoint_Write_Stream_LE(&data, sizeof(uint16_t));
		} else {
			Endpoint_ClearIN();
		}
	}

	Endpoint_SelectEndpoint(CTRL_IN_EPNUM);
	if (Endpoint_IsINReady() && Endpoint_IsReadWriteAllowed()) {
		gadget_err_t e;
		e.err = state->err;
		err = Endpoint_Write_Stream_LE(&e, sizeof(gadget_err_t));
	}

	Endpoint_SelectEndpoint(CTRL_OUT_EPNUM);
	if (Endpoint_IsOUTReceived() && Endpoint_IsReadWriteAllowed()) {
		usb_cmd_t cmd;
		err = Endpoint_Read_Stream_LE(&cmd, sizeof(usb_cmd_t));
		state->command = cmd.cmd_type;
		state->index   = cmd.index;
	}
}

#else

void StoreTask(state_t *state)
{
	uint8_t err;

	if (USB_DeviceState != DEVICE_STATE_Configured)
		return;

	Endpoint_SelectEndpoint(BULK_OUT_EPNUM);
	if (Endpoint_IsOUTReceived() && Endpoint_IsReadWriteAllowed()) {
		if(1) { // (CMD_STORE == state->command) {
			// err = Endpoint_Read_Stream_LE(&data, sizeof(uint16_t));
			uint8_t buf[BULK_EPSIZE];
			err = Endpoint_Read_Stream_LE(&buf, BULK_EPSIZE);
//			teensy_store(state->index, data);
		} else {
			Endpoint_ClearOUT();
		}

	}
}
#endif
