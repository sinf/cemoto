#ifndef F_CPU
#define F_CPU 9600000UL
#endif

#define PULSE_MAX 0xFF

typedef uint8_t pulse_t;
typedef uint8_t rtc_t; // only needs to count up to TICK_INTERVAL

#define PI 3.141592653589793

#define RTC_PRESCALER 64
#define ONE_SECOND (1.0*F_CPU/RTC_PRESCALER/256) // = 586

#define N_ACC 16

#define TICK_INTERVAL 15 // clear 8 bit timers (rtc, pulses)
#define TICK_INTERVAL_2 (TICK_INTERVAL*N_ACC) // calc speed 

#define WIPER_MAX 128

#define WHEEL_D (12 * 0.0254) // meters
#define WHEEL_R (WHEEL_D / 2)
#define PULSES_PER_REV 23
#define SENSOR_FREQ(veh_speed_kmh) ((veh_speed_kmh)/3.6/WHEEL_R/(2*PI)*PULSES_PER_REV)

#define THRES_KMH_LO 45
#define THRES_KMH_HI 60

#define FIX(fval,bits,t) ((t)((double)(fval)*(1L<<(bits)) + 0.5))

int16_t calc_speed(uint16_t pulse_acc, uint16_t rtc_acc)
{
	const int A = 4;
	typedef uint32_t T;

	const T freq_lo = FIX(SENSOR_FREQ(THRES_KMH_LO), A, T);
	const T freq_hi = FIX(SENSOR_FREQ(THRES_KMH_HI), A, T);
	//const T freq_lo = FIX(200, A, T);
	////const T freq_hi = FIX(1000, A, T);

	/*
	 freq = pulses / time
	      = pulses / (rtc / one_second)
	      = one_second * pulses / rtc
	*/

	T freq = FIX(ONE_SECOND,A,T) * pulse_acc / rtc_acc;
	// freq is now fixed point frequency in Hz

	if (freq <= freq_lo) {
		return 0;
	} else if (freq >= freq_hi) {
		return WIPER_MAX;
	} else {
		int16_t y = ( freq - freq_lo ) * WIPER_MAX / ( freq_hi - freq_lo );
		if (y < 0) y = 0;
		if (y > WIPER_MAX) y = WIPER_MAX;
		return y;
		//return WIPER_MAX - y;
	}
}

