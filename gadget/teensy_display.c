// teensy_main.c
// Author : A. John Ochsner IV
// Group  : Rebekah Leslie & Thomas DuBuisson

#include <string.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <stdint.h>

// copied from blinky.h @ www.pjrc.com
#define CPU_PRESCALE(n)	(CLKPR = 0x80, CLKPR = (n))
#define LED_CONFIG	(DDRD |= (1<<6))
#define LED_ON		(PORTD &= ~(1<<6))
#define LED_OFF		(PORTD |= (1<<6))

char * test = "0123456789ABCDEF";  // want to use PSTR("")
unsigned int length = 0;

unsigned short hex_val(const char c)
  {
  switch(c)
    {
    case '0':return 0;    
    case '1':return 1;
    case '2':return 2;
    case '3':return 3;
    case '4':return 4;
    case '5':return 5;
    case '6':return 6;
    case '7':return 7;
    case '8':return 8;
    case '9':return 9;
    case 'A':return 10;
    case 'B':return 11;
    case 'C':return 12;
    case 'D':return 13;
    case 'E':return 14;
    case 'F':return 15;
    default :return 0;
    }
  }

void digitalWrite(uint8_t pin, uint8_t val)
  {
  switch(pin)
    {
    case 0 : if (val)
               PORTB |= (1);
             else
               PORTB &= ~(1);
             break;
    case 1 : if (val)
               PORTB |= (1<<1);
             else
               PORTB &= ~(1<<1);
             break;
    case 2 : if (val)
               PORTB |= (1<<2);
             else
               PORTB &= ~(1<<2);
             break;
    case 3 : if (val)
               PORTB |= (1<<3);
             else
               PORTB &= ~(1<<3);
             break;
    case 4 : if (val)
               PORTB |= (1<<7);
             else
               PORTB &= ~(1<<7);
             break;
    case 5 : if (val)
               PORTD |= (1);
             else
               PORTD &= ~(1);
             break;
    case 6 : if (val)
               PORTD |= (1<<1);
             else
               PORTD &= ~(1<<1);
             break;
    case 7 : if (val)
               PORTD |= (1<<2);
             else
               PORTD &= ~(1<<2);
             break;
    case 14: if (val)
               PORTB |= (1<<5);
             else
               PORTB &= ~(1<<5);
             break;
    case 15: if (val)
               PORTB |= (1<<6);
             else
               PORTB &= ~(1<<6);
             break;
    case 16: if (val)
               PORTF |= (1<<7);
             else
               PORTF &= ~(1<<7);
             break;
    case 17: if (val)
               PORTF |= (1<<6);
             else
               PORTF &= ~(1<<6);
             break;
    case 18: if (val)
               PORTF |= (1<<5);
             else
               PORTF &= ~(1<<5);
             break;
    case 19: if (val)
               PORTF |= (1<<4);
             else
               PORTF &= ~(1<<4);
             break;
    case 20: if (val)
               PORTF |= (1<<1);
             else
               PORTF &= ~(1<<1);
             break;
    case 21: if (val)
               PORTF |= (1);
             else
               PORTF &= ~(1);
             break;
    default: break;
    }
  return;
  }
  
void out_hex(unsigned short nib, unsigned short pos)
  {
  uint8_t signal;
  unsigned short i = 0;
  
  pos *= 4;
  if (pos > 7)
    pos += 6;
  for(i=0 ; i<4 ; i++)
    {
//    signal = hex_dig(nib, i);
    signal = nib;
    signal &= (1<<i);
    
    if (signal)
      digitalWrite(pos+i, 1);
    else
      digitalWrite(pos+i, 0);
    }
  }

void Display_Init(void)
{
  unsigned short i = 0;

  length = strlen(test);

  DDRB = 0xef;  // all port B pins except PB4 are outputs
  DDRD = 0x7;   // port D pins 0, 1, && 2 are outputs
  DDRF = 0xf3;  // all port F pins (0,1,4,5,6,7) are outputs

  // initialize all 4 7-segments to display 0
  for (i=0 ; i<8 ; i++)
    digitalWrite(i, 0);
  for (i=14 ; i<22 ; i++)
    digitalWrite(i, 0);
}
