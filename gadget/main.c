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
	int index;
	int offset;
	int command;
	uint8_t err;
};

typedef struct state state_t;

void StoreTask(state_t *state);
void Init(void);


void EVENT_USB_Device_ConfigurationChanged(void)
{
	int i,succ=1;

#ifdef CTRL
	succ &= Endpoint_ConfigureEndpoint(CTRL_IN_EPNUM, EP_TYPE_CONTROL,
					ENDPOINT_DIR_IN, CTRL_IN_EPSIZE,
					ENDPOINT_BANK_SINGLE);

	succ &= Endpoint_ConfigureEndpoint(CTRL_OUT_EPNUM, EP_TYPE_CONTROL,
					ENDPOINT_DIR_OUT, CTRL_OUT_EPSIZE,
					ENDPOINT_BANK_SINGLE);
#endif

	succ &= Endpoint_ConfigureEndpoint(BULK_IN_EPNUM, EP_TYPE_BULK,
					ENDPOINT_DIR_IN, BULK_EPSIZE,
					ENDPOINT_BANK_SINGLE);

	succ &= Endpoint_ConfigureEndpoint(BULK_OUT_EPNUM, EP_TYPE_BULK,
					ENDPOINT_DIR_OUT, BULK_EPSIZE,
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
	LED_CONFIG;
	LED_OFF;
//	Display_Init();
}

void StoreTask(state_t *state)
{
#ifdef CTRL
	gadget_err_t e;
	usb_cmd_t cmd;
#endif
	uint8_t err;
	static uint16_t data=0;

	if (USB_DeviceState != DEVICE_STATE_Configured)
		return;

	Endpoint_SelectEndpoint(BULK_OUT_EPNUM);
	if (Endpoint_IsConfigured() && Endpoint_IsOUTReceived() && Endpoint_IsReadWriteAllowed()) {
		if (CMD_STORE == state->command) {
			err = Endpoint_Read_Stream_LE(&data, sizeof(uint16_t));
			// teensy_store(state->index, data);		//	FIXME
			LED_OFF;
		} else {
			Endpoint_ClearOUT();
		}
	}

	Endpoint_SelectEndpoint(BULK_IN_EPNUM);
	if (Endpoint_IsConfigured() && Endpoint_IsINReady() && Endpoint_IsReadWriteAllowed()) {
		if(CMD_READ == state->command) {
			// teensy_read(state->index, &data);		//	FIXME
			err = Endpoint_Write_Stream_LE(&data, sizeof(uint16_t));
			LED_OFF;
		} else {
			Endpoint_ClearIN();
		}
	}

#ifdef CTRL
	Endpoint_SelectEndpoint(CTRL_IN_EPNUM);
	if (Endpoint_IsConfigured() && Endpoint_IsINReady() && Endpoint_IsReadWriteAllowed()) {
		e.err = state->err;
		err = Endpoint_Write_Stream_LE(&e, sizeof(gadget_err_t));
	}

	Endpoint_SelectEndpoint(CTRL_OUT_EPNUM);
	if (Endpoint_IsConfigured() && Endpoint_IsOUTReceived() && Endpoint_IsReadWriteAllowed()) {
		LED_ON;
		err = Endpoint_Read_Stream_LE(&cmd, sizeof(usb_cmd_t));
		state->command = cmd.cmd_type;
		state->index   = cmd.index;
	}
#endif
}
