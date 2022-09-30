#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/cpufunc.h>
#include <util/delay.h>
#include <util/delay_basic.h>
#include "lcd.h"
#include "odo.h"

void set_num(uint8_t off, uint8_t n, uint16_t value) {
	for(uint8_t i=0; i<n; ++i) {
		lcd_mem[off+n-1-i] = lcd_num(value % 10);
		value /= 10;
	}
}

void show_clock(void) {
	uint16_t s = uptime_sec;
	set_num(2, 2, s % 60);
	set_num(0, 2, s / 60);
	lcd_mem[0] |= SEGDP; // colon
}

void show_odo(void) {
	set_num(0, 4, trip_100m);
	lcd_mem[3] |= SEGDP; // decimal point

	// drop leading zeros
	if (lcd_mem[0] == SEG_0) {
		lcd_mem[0] = 0;
		if (lcd_mem[1] == SEG_0) {
			lcd_mem[1] = 0;
		}
	}
}

int main(void)
{
	DDRB = LCD_CS|LCD_WR|LCD_DAT; // outputs
	PORTB = LCD_CS|HALL|PUSHBUTTON; // pull-up enabled for hall + button
	
	/* setup timer interrupt */
	OCR0A = 16;
	TCCR0A = 1<<WGM01; // dont toggle hardware pin, CTC mode
	TCCR0B = 1<<CS00 | 1<<CS01; // prescaler = 64
	TIMSK0 = 1<<OCIE0A; // compare match A interrupt enable
	//TIMSK0 = 1<<TOIE0; // enable overflow interrupt for timer 0

	/* setup pin change interrupt */
	PCMSK = 1<<PCINT3; // PCINT3=hall, PCINT4=button
	GIMSK = 1<<PCIE;

	//_delay_ms(1);
	lcd_init();
	odo_load();
	sei();

	uint8_t button = 0;

	for(uint8_t tick=0;; ++tick)
	{
		//lcd_mem[0] = lcd_mem[1] = lcd_mem[2] = lcd_mem[3] = lcd_num(num);
		//lcd_mem[0] |= SEGDP; //colon

		if (tick & 0x40) {
			show_clock();
		} else {
			show_odo();
		}

		if (tick == 200) {
			// save sometimes. 20 Hz 8-bit counter overflows every 12.8 s
			odo_save();
		}

		lcd_refresh();
		odo_tick();
		_delay_ms(50);

		button = (~PINB & PUSHBUTTON) ? (button + 1) : 0;
		if (button == 100) {
			trip_100m = 0;
		}
	}

	return 0;
}


