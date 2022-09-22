#ifndef F_CPU
#define F_CPU 96000000UL
#endif

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <util/delay_basic.h>

#define WIPER_MAX 128

int16_t wiper_pos = WIPER_MAX/2;

#define RTC_PRESCALER 64
#define ONE_SECOND (rtc_t)((F_CPU/RTC_PRESCALER/256)) // = 586
typedef int16_t rtc_t;
volatile rtc_t rtc_counter = 0;
ISR(TIM0_OVF_vect) { rtc_counter += 1; }

// digital potentiometer pins
// max spi freq = 10 MHz
#define SDI (1u<<0)
#define SCK (1u<<1)
#define CS (1u<<2)
#define spi_delay() _delay_loop_1(100)
#define spi_delay2() _delay_loop_1(255)

#define HALL (1u<<3)
#define LED (1u<<4)

void boot_anim() {
	for(int i=0; i<5; ++i) {
		PORTB ^= LED;
		_delay_ms(100);
	}
	PORTB = PORTB & (~LED);
}

#define CMD_8(addr,cmd,data) ((addr)<<4 | (cmd)<<2 & 12 | (data) & 3)
#define CMD_16(addr,cmd,data) ((uint16_t) CMD_8((addr),(cmd),(uint16_t)(data) >> 8) << 8 | (uint8_t)(data) & 0xFFu)

// cmd bits
#define CMD_WRITE 0
#define CMD_INCR 1
#define CMD_DECR 2
#define CMD_READ 3

#define WIPER0 0
#define WIPER1 1

void spi_send(uint16_t data, uint8_t bits)
{
	PORTB &= ~CS;
	spi_delay2();

	for(uint8_t i=0; i<bits; ++i) {

		uint8_t data_bit = data >> bits - 1 - i & 1;

		PORTB = PORTB & ~(SCK | SDI);
		PORTB |= data_bit;
		spi_delay();

		PORTB |= SCK;
		spi_delay();
	}

	spi_delay2();
	PORTB |= CS;
}

void inc_wiper(int8_t delta) {
	spi_send(CMD_8(WIPER0, delta > 0 ? CMD_INCR : CMD_DECR, 0), 8);
}

void set_wiper(int16_t value) {
	if (value < 0) { value = 0; }
	if (value > WIPER_MAX) { value = WIPER_MAX; }
	wiper_pos = value;

	spi_send(CMD_16(WIPER0, CMD_WRITE, value), 16);
}

void do_tick() {
	static int16_t d = 1;
	set_wiper(wiper_pos + d);
	if (wiper_pos == 0 || wiper_pos == WIPER_MAX) {
		d = -d;
	}
}

int main(void)
{
	/*
	 * PB0: data of digi pot (SDI/SDO/SI)
	 * PB1: SCK of digi pot
	 * PB2: CS of digi pot
	 * PB3: hall sensor
	 * PB4: output LED
	 * DDRB 1-bits mean outputs
	 * PORTB 1-bits for inputs enables pull up
	 */
	DDRB = LED | SCK | CS | SDI;
	PORTB = HALL | CS;
	
	/* setup timer interrupt */
	TCCR0A = 0; // dont toggle hardware pin, normal mode
	TCCR0B = 1<<CS00 | 1<<CS01; // prescaler = 64
	TIMSK0 = 1<<TOIE0; // enable overflow interrupt for timer 0

	boot_anim();
	PORTB |= LED;
	
	set_wiper(wiper_pos);

	if (0) for(;;) {
		set_wiper(10);
		_delay_ms(100);
		set_wiper(240);
		_delay_ms(100);
	}

	const rtc_t tick_interval = 10;
	for(;;) {
		sei();
		while (rtc_counter < tick_interval) { }
		cli();
		if (rtc_counter < tick_interval) { continue; } // second check incase the while loop misread the counter
		do_tick();
	}

	return 0;
}

