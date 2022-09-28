#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/cpufunc.h>
#include <util/delay.h>
#include <util/delay_basic.h>
#include "lcd.h"
#include "odo.h"

int main(void)
{
	DDRB = LCD_CS|LCD_WR|LCD_DAT; // outputs
	PORTB = LCD_CS|HALL|PUSHBUTTON; // pull-up enabled for hall + button
	
#if 0
	/* setup timer interrupt */
	TCCR0A = 0; // dont toggle hardware pin, normal mode
	TCCR0B = 1<<CS00 | 1<<CS02; // prescaler = 1024
	TIMSK0 = 1<<TOIE0; // enable overflow interrupt for timer 0
#endif

	_delay_ms(1);
	lcd_init();

	uint8_t num = 0;
	uint8_t dot = 0;

	sei();

	for(;;)
	{
		lcd_mem[0] = lcd_mem[1] = lcd_mem[2] = lcd_mem[3] = lcd_num(num);
		//lcd_mem[0] |= SEGDP; //semicolon

		dot = (dot + 1) & 3;
		lcd_mem[dot] |= SEGDP; //semicolon

		lcd_refresh();
		_delay_ms(500);
		if (++num == 10) num = 0;
	}

	return 0;
}


