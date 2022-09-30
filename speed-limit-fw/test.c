#include <stdint.h>
#include <stdio.h>
#include "speed.c"

#define UNIT_TEST

#define TEST_LH(p, t, lo, hi) \
	result=calc_speed((p),(t)); \
	printf("%s:%-4d: calc_speed(%6d,%6d) = %-8d [%s]\n", \
			__FILE__, __LINE__, (int)(p), (int)(t), (int)(result), \
			(result >= (lo) && result <= (hi)) ? "PASS" : "** FAIL **")

#define TEST(p, t, x) TEST_LH(p,t,x,x)

#define TEST_KMH(kmh, dt, val, err) \
	TEST_LH(SENSOR_FREQ(kmh)*dt, ONE_SECOND*dt, val-err, val+err);

#define fp_print(name, value, bits) \
	printf("fixed point [%s] = %.5g = %08x\n", name, value, FIX((value),(bits),unsigned))

void ovf_test() {
	typedef uint64_t T;
	const int A = 8;
	T pulse_acc = N_ACC * PULSE_MAX;
	T rtc_acc = N_ACC * TICK_INTERVAL * 1.1;
	T sec = FIX(ONE_SECOND,A,T);
	T freq = sec*pulse_acc/rtc_acc;
	const T freq_lo = FIX(SENSOR_FREQ(THRES_KMH_LO), A, T);

	printf("\n");
	printf("pulse_acc = %x\n", pulse_acc);
	printf("rtc_acc = %x\n", rtc_acc);
	printf("FIX(ONE_SECOND,%d,T) * pulse_acc = %x * %x = %x\n", A, sec, pulse_acc, sec*pulse_acc);
	printf("freq = %x\n", freq);
	printf("(freq - freq_lo) * WIPER_MAX = %x\n", (freq - freq_lo) * WIPER_MAX);
	printf("\n");
}

int main() {
	int16_t result=0;

	printf("testing\n");

	const int A = 8;
	fp_print("freq_lo", SENSOR_FREQ(THRES_KMH_LO), A);
	fp_print("freq_lo", SENSOR_FREQ(THRES_KMH_HI), A);
	fp_print("freq_delta", SENSOR_FREQ(THRES_KMH_HI) - SENSOR_FREQ(THRES_KMH_LO), A);

	const double TICKS_PER_SEC = ONE_SECOND / (double) TICK_INTERVAL_2;

	printf("fast ticks/second: %.5g\n", ONE_SECOND/TICK_INTERVAL);
	printf("filtered ticks/second: %.5g\n", TICKS_PER_SEC);
	printf("filtered seconds/tick: %.5g\n", 1.0/TICKS_PER_SEC);

	printf("pulse counter at 10 km/h: %.5g\n", SENSOR_FREQ(10) / TICKS_PER_SEC);
	printf("pulse counter at 50 km/h: %.5g\n", SENSOR_FREQ(50) / TICKS_PER_SEC);
	printf("pulse counter range at THRES_KMH_LO ... THRES_KMH_HI: %.5g\n",
			SENSOR_FREQ(THRES_KMH_HI) / TICKS_PER_SEC
			- SENSOR_FREQ(THRES_KMH_LO) / TICKS_PER_SEC);
	printf("pulse counter at 60 km/h: %.5g\n", SENSOR_FREQ(60) / TICKS_PER_SEC);
	printf("pulse counter at 100 km/h: %.5g\n", SENSOR_FREQ(100) / TICKS_PER_SEC);
	printf("pulse counter at 200 km/h: %.5g\n", SENSOR_FREQ(200) / TICKS_PER_SEC);

	printf("pulses/second at 10 km/h: %.5g\n", SENSOR_FREQ(10));
	printf("pulses/second at 50 km/h: %.5g\n", SENSOR_FREQ(50));
	printf("pulses/second at 60 km/h: %.5g\n", SENSOR_FREQ(60));
	printf("pulses/second at 100 km/h: %.5g\n", SENSOR_FREQ(100));
	printf("pulses/second at 200 km/h: %.5g\n", SENSOR_FREQ(200));

	printf("pulses/second at THRES_KMH_LO: %.5g\n", SENSOR_FREQ(THRES_KMH_LO));
	printf("pulses/second at THRES_KMH_HI: %.5g\n", SENSOR_FREQ(THRES_KMH_HI));

	ovf_test();

	// no speed
	TEST(0,1,0);
	TEST(0,10,0);
	TEST(0,1000,0);
	TEST(0,0xFFFF,0);

	// one rpm
	TEST(PULSES_PER_REV, ONE_SECOND, WIPER_MAX);

	TEST_KMH(20, 0.1, WIPER_MAX, 0);
	TEST_KMH(20, 0.05, WIPER_MAX, 0);
	TEST_KMH(20, 0.01, WIPER_MAX, 0);

	TEST_KMH(30, 0.1, WIPER_MAX, 0);
	TEST_KMH(30, 0.05, WIPER_MAX, 0);
	TEST_KMH(30, 0.01, WIPER_MAX, 0);

	TEST_KMH(45, 0.1, WIPER_MAX, 0);
	TEST_KMH(45, 0.05, WIPER_MAX, 0);
	TEST_KMH(45, 0.01, WIPER_MAX, 0);

	const int tol = 10;

	TEST_KMH(THRES_KMH_LO, 0.1, WIPER_MAX, tol);
	TEST_KMH(THRES_KMH_LO, 0.05, WIPER_MAX, tol);
	TEST_KMH(THRES_KMH_LO, 0.01, WIPER_MAX, tol);

	// half speed
	const double half = ( THRES_KMH_LO + THRES_KMH_HI ) / 2;
	TEST_KMH(half, 2, WIPER_MAX/2, tol);
	TEST_KMH(half, 0.5, WIPER_MAX/2, tol);
	TEST_KMH(half, 0.01, WIPER_MAX/2, tol);

	TEST_KMH(THRES_KMH_HI, 2, 0, tol);
	TEST_KMH(THRES_KMH_HI, 0.5, 0, tol);
	TEST_KMH(THRES_KMH_HI, 0.01, 0, tol);

	return 0;
}

