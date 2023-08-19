
// ****************************************************************************
//
//                                  Timer
//                         Precise Timer with Alarm
//
// ****************************************************************************
// PicoLibSDK - Alternative SDK library for Raspberry Pico and RP2040
// Copyright (c) 2023 Miroslav Nemecek, Panda38@seznam.cz, hardyplotter2@gmail.com
// 	https://github.com/Panda381/PicoLibSDK
//	https://www.breatharian.eu/hw/picolibsdk/index_en.html
//	https://github.com/pajenicko/picopad
//	https://picopad.eu/en/
// License:
//	This source code is freely available for any purpose, including commercial.
//	It is possible to take and modify the code or parts of it, without restriction.

#include "../../global.h"	// globals

#if USE_TIMER	// use Timer with alarm (sdk_timer.c, sdk_timer.h)

#include "../inc/sdk_timer.h"
#include "../inc/sdk_cpu.h"

// get 64-bit absolute system time in [us] - fast method, but it is not atomic safe
//   - do not use simultaneously from both processor cores and from interrupts
u64 Time64Fast()
{
	u32 low = *TIMER_TIMELR; // get latched time LOW
	cb(); // compiler barrier (or else the compiler could swap the order of operations)
	u32 high = *TIMER_TIMEHR; // get latched time HIGH
	return ((u64)high << 32) | low;
}

// get 64-bit absolute system time in [us] - atomic method (concurrently safe)
u64 Time64()
{
	u32 low, high;
	u32 high2 = *TIMER_TIMERAWH;  // get raw time HIGH

	do {
		// accept new time HIGH
		high = high2;

		// get raw time LOW
		cb(); // compiler barrier (or else the compiler could swap the order of operations)
		low = *TIMER_TIMERAWL;

		// get raw time HIGH again
		cb(); // compiler barrier (or else the compiler could swap the order of operations)
		high2 = *TIMER_TIMERAWH;

	// check that HIGH has not changed, otherwise a re-read will be necessary
	} while (high != high2);

	// return result value
	return ((u64)high << 32) | low;
}

// wait for delay in [us] (max. 71 minutes, 4'294'967'295 us)
void WaitUs(u32 us)
{
	u32 start = Time();
	while ((u32)(Time() - start) < us) {}
}

// wait for delay in [ms] (max. 71 minutes, 4'294'967 ms)
void WaitMs(int ms)
{
	WaitUs((u32)ms*1000);
}

// start alarm
//   alarm = alarm number 0..3
//   handler = interrupt handler
//   time = time interval in [us] after which to activate first alarm (min 0 us, max 71 minutes)
// Vector table must be located in RAM
// If vector table is not in RAM, use services with names isr_timer_0..isr_timer_3
// Call AlarmAck on start of interrupt, then AlarmRestart to restart again, or AlarmStop to deactivate.
// Alarm handler is shared between both processor cores. Only alarms of different numbers can be independent.
void AlarmStart(u8 alarm, irq_handler_t handler, u32 time)
{
	// stop previous alarm
	AlarmStop(alarm);

	// prepare interrupt IRQ
	u8 irq = ALARM_IRQ(alarm);

	// set interrupt handler to VTOR interrupt table (VTOR should be in RAM)
	SetHandler(irq, handler);

	// enable interrupt on NVIC
	NVIC_IRQEnable(irq);

	// enable interrupt from the timer
	RegSet(TIMER_INTE, BIT(alarm));

	// arm timer
	if (time < 3) time = 3;
	IRQ_LOCK;
	*TIMER_ALARM(alarm) = time + Time();
	IRQ_UNLOCK;
}

// restart alarm - can be called from an interrupt for a repeated alarm
//   time = time interval in [us] after which to activate next alarm (max 71 minutes)
// The maximum achievable interrupt frequency is about 100 kHz.
void AlarmRestart(u8 alarm, u32 time)
{
	// re-arm timer
	IRQ_LOCK;
	time += *TIMER_ALARM(alarm); // next absolute time
	u32 t = Time(); // current time
	s32 dt = (s32)(time - t); // remaining time
	if ((dt < 4) && (dt > -50)) time = t + 4;
	*TIMER_ALARM(alarm) = time;
	IRQ_UNLOCK;
}

// stop alarm - can be called from an interrupt if no next interrupt is required
void AlarmStop(u8 alarm)
{
	// disable interrupt from the timer
	RegClr(TIMER_INTE, BIT(alarm));

	// disarm alarm
	*TIMER_ARMED = BIT(alarm);

	// acknowledge alarm request
	AlarmAck(alarm);

	// disable interrupt on NVIC
	NVIC_IRQDisable(ALARM_IRQ(alarm));
}

#endif // USE_TIMER	// use Timer with alarm (sdk_timer.c, sdk_timer.h)
