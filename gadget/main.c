#include <string.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <avr/power.h>
#include <util/delay.h>
#include <stdint.h>

//#include <LUFA/Drivers/USB/LowLevel/Endpoint.h>
#include <LUFA/Drivers/USB/USB.h>
//#include <LUFA/Drivers/USB/HighLevel/StdRequestType.h>

#include "teensy_blobs.h"
#include "teensy_display.h"
#include "desc.h"
#include "../commands.h"

int config_led = 0;

#define CPU_PRESCALE(n) (CLKPR = 0x80, CLKPR = (n))
#define LED_CONFIG      if(0 == config_led) { config_led = 1 ; DDRD |= (1<<6); }
#define LED_OFF         (PORTD &= ~(1<<6))
#define LED_ON          (PORTD |= (1<<6))

struct state {
	uint8_t command;
	uint64_t index;
};

typedef struct state state_t;

void StoreTask(state_t *state);
void Init(void);


void EVENT_USB_Device_ConfigurationChanged(void)
{
	int i,succ=1;

	succ &= Endpoint_ConfigureEndpoint(BULK_IN_EPNUM, EP_TYPE_BULK,
					ENDPOINT_DIR_IN, BULK_IN_EPSIZE,
					ENDPOINT_BANK_SINGLE);

	succ &= Endpoint_ConfigureEndpoint(BULK_OUT_EPNUM, EP_TYPE_BULK,
					ENDPOINT_DIR_OUT, BULK_OUT_EPSIZE,
					ENDPOINT_BANK_SINGLE);

	succ &= Endpoint_ConfigureEndpoint(COMMAND_EPNUM, EP_TYPE_BULK,
					ENDPOINT_DIR_OUT, COMMAND_EPSIZE,
					ENDPOINT_BANK_SINGLE);
	LED_CONFIG;
	while(!succ) {
		LED_ON;
		_delay_ms(1000);
		LED_OFF;
		_delay_ms(1000);
	}
	for(i = 0; i < 5; i ++) {
		LED_ON;
		_delay_ms(50);
		LED_OFF;
		_delay_ms(50);
	}
}

void EVENT_USB_Device_UnhandledControlRequest(void)
{
	while(1) {
		LED_ON;
		_delay_ms(2000);
		LED_OFF;
		_delay_ms(2000);
	}
}

int main(void)
{
	state_t state;
	state.command = CMD_NULL;
	state.index = 0;

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
	LED_CONFIG;
	LED_OFF;
//	Display_Init();
}

void StoreTask(state_t *state)
{
	uint8_t err;

	if (USB_DeviceState != DEVICE_STATE_Configured)
		return;

	Endpoint_SelectEndpoint(BULK_IN_EPNUM);
	if (Endpoint_IsConfigured() && Endpoint_IsINReady() && Endpoint_IsReadWriteAllowed()) {
		if(CMD_READ == state->command) {
			uint16_t data;
			teensy_read(state->index, &data);
			err = Endpoint_Write_Stream_LE((void *)&data, sizeof(uint16_t));
		}
		Endpoint_ClearIN();
	}

	Endpoint_SelectEndpoint(BULK_OUT_EPNUM);
	if (Endpoint_IsConfigured() && Endpoint_IsOUTReceived() && Endpoint_IsReadWriteAllowed()) {
		if(CMD_STORE == state->command) {
			uint16_t data;
			err = Endpoint_Read_Stream_LE(&data, sizeof(uint16_t));
			teensy_store(state->index, data);
		}
		Endpoint_ClearOUT();
	}

	Endpoint_SelectEndpoint(COMMAND_EPNUM);
	if (Endpoint_IsConfigured() && Endpoint_IsOUTReceived() && Endpoint_IsReadWriteAllowed()) {
		usb_cmd_t cmd;
		err = Endpoint_Read_Stream_LE(&cmd, sizeof(usb_cmd_t));
		switch (cmd.cmd_type) {
			case CMD_DELETE:
				teensy_delete(cmd.index);
				break;
			default:
				;
		}
		state->command = cmd.cmd_type;
		state->index   = cmd.index;
		Endpoint_ResetFIFO(BULK_IN_EPNUM);
		Endpoint_ResetFIFO(BULK_OUT_EPNUM);
		Endpoint_ClearOUT();
	}
}
