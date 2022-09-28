#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/cpufunc.h>
#include <util/delay.h>
#include <util/delay_basic.h>
#include "odo.h"

volatile uint32_t rtc_counter = 0;
volatile uint32_t pulse_counter = 0;
volatile uint8_t button_press = 0;

uint16_t trip_m = 0;
uint16_t uptime_sec = 0;

ISR(TIM0_OVF_vect) {
	rtc_counter += 1;
}

ISR(PCINT0_vect) {

	static uint8_t prev = 0;
	uint8_t cur = PINB;
	uint8_t falling_edge = prev & ~cur;

	if (falling_edge & PUSHBUTTON) {
		button_press = 1;
	}

	if (falling_edge & HALL) {
		pulse_counter += 1;
	}
}

void odo_tick(void)
{
	uint32_t rtc, pulses;

	cli();
	rtc = rtc_counter;
	pulses = pulse_counter;
	sei();

	uint32_t B = 8;
	trip_m = ROUND(FIX(2*PI*WHEEL_R,B,uint32_t) * pulses / PULSES_PER_REV, B);
	uptime_sec = (rtc + RTC_FREQ/2) / RTC_FREQ;
}

