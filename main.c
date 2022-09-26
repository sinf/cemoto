#ifndef F_CPU
#define F_CPU 9600000UL
#endif

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <util/delay_basic.h>

// digital potentiometer pins
// max spi freq = 10 MHz
#define SDI (1u<<0)
#define SCK (1u<<1)
#define CS (1u<<2)
#define spi_delay() _delay_loop_1(10)
#define spi_delay2() _delay_loop_1(30)

#define HALL (1u<<3)
#define LED (1u<<4)

#include "speed.c"

int16_t wiper_pos = WIPER_MAX;
volatile pulse_t pulse_counter = 0; // should increment at rate < 1000 Hz
volatile rtc_t rtc_counter = 0;

ISR(TIM0_OVF_vect) {
	rtc_counter += 1;
}

ISR(PCINT0_vect) {
	if (pulse_counter != PULSE_MAX)
		pulse_counter += 1;
}

#if 0
#define boot_anim()
#else
void boot_anim() {
	for(int i=0; i<5; ++i) {
		PORTB ^= LED;
		_delay_ms(100);
	}
	PORTB = PORTB & (~LED);
}
#endif

#define CMD_8(addr,cmd,data) ((addr)<<4 | (cmd)<<2 & 12 | (data) & 3)
#define CMD_16(addr,cmd,data) ((uint16_t) CMD_8((addr),(cmd),(uint16_t)(data) >> 8) << 8 | (uint8_t)(data) & 0xFFu)

// cmd bits
#define CMD_WRITE 0
#define CMD_INCR 1
#define CMD_DECR 2
#define CMD_READ 3

// addresses
#define WIPER0 0
#define WIPER1 1
#define TCON 2
#define STATUS 5

void spi_send(uint16_t data, uint8_t bits)
{
	data <<= 16 - bits;
	PORTB &= ~CS;
	spi_delay2();

	for(uint8_t i=0; i<bits; ++i) {

		uint8_t data_bit = data << i >> 15;

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

#if 1
	if (value > 0) {
		PORTB |= LED;
	} else {
		PORTB &= ~LED;
	}
#endif

	spi_send(CMD_16(TCON, CMD_WRITE, 0x1FF), 16); // just in case a random bit flip corrupts TCON
	spi_send(CMD_16(WIPER0, CMD_WRITE, value), 16);
}

#if 0
void test_wiper() {
	int16_t w = 0;
	int16_t d = 1;
	for(;;) {
		set_wiper(w);
		w += d;
		if (w == 0 || w == WIPER_MAX) {
			d = -d;
		}
	}
}
#endif

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
	PORTB = CS | HALL; // pull ups
	
	/* setup timer interrupt */
	TCCR0A = 0; // dont toggle hardware pin, normal mode
	TCCR0B = 1<<CS00 | 1<<CS01; // prescaler = 64
	TIMSK0 = 1<<TOIE0; // enable overflow interrupt for timer 0

	/* setup pin change interrupt */
	PCMSK = 1<<PCINT3;
	GIMSK = 1<<PCIE;

	boot_anim();
	//PORTB |= LED;
	
	set_wiper(wiper_pos);

	rtc_t rtc_acc[N_ACC] = {0};
	pulse_t pulse_acc[N_ACC] = {0};
	uint8_t i_acc = 0;

	sei();

	for(;;) {
		rtc_t rtc_copy;
		pulse_t pulse_copy;

		while (rtc_counter < TICK_INTERVAL) { }
		cli();
		rtc_copy = rtc_counter;
		pulse_copy = pulse_counter;
		rtc_counter = 0;
		pulse_counter = 0;
		sei();

		rtc_acc[i_acc] = rtc_copy;
		pulse_acc[i_acc] = pulse_copy;
		if (++i_acc == N_ACC) {
			i_acc = 0;
		}

		uint16_t t=0, p=0;
		for(int j=0; j<N_ACC; ++j) {
			t += rtc_acc[j];
			p += pulse_acc[j];
		}

		//t <<= 1; // adjust because counted all pulses twice
		p >>= 1;

		int16_t w = calc_speed(p, t);
		set_wiper(w);
	}

	return 0;
}

