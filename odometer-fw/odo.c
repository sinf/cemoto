#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include "odo.h"

#define TRIP_SLOTS 31

typedef struct {
	uint16_t trip_100m[TRIP_SLOTS];
	uint16_t slot;
} eep_data_t;
static eep_data_t *ee = 0;
static uint8_t cur_slot_writes;

static volatile uint16_t rtc_counter=0;
static volatile uint16_t pulse_counter=0;
uint16_t uptime_sec=0;
uint16_t trip_100m;

ISR(TIM0_COMPA_vect) {
	static uint16_t t=0;
	if (++t >= RTC_FREQ) {
		t = 0;
		rtc_counter += 1;
	}
}

ISR(PCINT0_vect) {
	pulse_counter += 1;
}

static uint8_t cur_slot(void) {
	uint8_t s = eeprom_read_word(&ee->slot);
	return s < TRIP_SLOTS ? s : 0;
}

void odo_load(void)
{
	trip_100m = eeprom_read_word(ee->trip_100m + cur_slot());
}

void odo_save(void)
{
	uint8_t slot = cur_slot();

	if (cur_slot_writes >= TRIP_SLOTS) {
		if (++slot >= TRIP_SLOTS) {
			slot = 0;
		}
		cur_slot_writes = 0;
		eeprom_update_word(&ee->slot, slot);
	}

	eeprom_update_word(ee->trip_100m + slot, trip_100m);
}

void odo_tick(void)
{
	//*2 because 2 pin changes per pulse
	const uint16_t pulses_100m = 100 / (2*PI*WHEEL_R) * PULSES_PER_REV * 2 + 0.5;

	cli();
	uptime_sec = rtc_counter;
	if (pulse_counter >= pulses_100m) {
		pulse_counter -= pulses_100m;
		trip_100m += 1;
	}
	sei();
}

