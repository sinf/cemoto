#ifndef HALL_PIN

#include <stdint.h>

#define FIX(fval,bits,t) ((t)((double)(fval)*(1L<<(bits)) + 0.5))
#define ROUND(fp,bits) (((fp) + (1<<(bits))/2)>>(bits))

#define PI 3.141592653589793

#define RTC_PRESCALER 1024
#define RTC_FREQ (F_CPU/RTC_PRESCALER)

#define HALL_PIN 3
#define HALL (1u<<HALL_PIN)

#define PUSHBUTTON_PIN 4
#define PUSHBUTTON (1u<<PUSHBUTTON_PIN)

#define WHEEL_D (12 * 0.0254) // meters
#define WHEEL_R (WHEEL_D / 2)
#define PULSES_PER_REV 23
#define SENSOR_FREQ(veh_speed_kmh) ((veh_speed_kmh)/3.6/WHEEL_R/(2*PI)*PULSES_PER_REV)

extern volatile uint32_t rtc_counter;
extern volatile uint32_t pulse_counter;
extern volatile uint8_t button_press;

extern uint16_t trip_m;
extern uint16_t uptime_sec;

void odo_tick(void);

#endif

