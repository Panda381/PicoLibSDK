
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
#include "../inc/sdk_spinlock.h"

// ----------------------------------------------------------------------------
//                     Microsecond timer (RP2350 uses TIMER0)
// ----------------------------------------------------------------------------

// claimed alarms
Bool AlarmClaimed[ALARMS_NUM];	// claimed alarms

#if USE_ORIGSDK		// include interface of original SDK
static hardware_alarm_callback_t alarm_callbacks[NUM_TIMERS] = { NULL, NULL, NULL, NULL };
static u32 target_hi[NUM_TIMERS];
static u8 timer_callbacks_pending = 0;
static alarm_callback_t timer_callbacks[NUM_TIMERS] = { NULL, NULL, NULL, NULL };
static void* timer_data[NUM_TIMERS] = { NULL, NULL, NULL, NULL };
#endif // USE_ORIGSDK

// get 64-bit absolute system time in [us] - fast method, but it is not atomic safe
//   - do not use simultaneously from both processor cores and from interrupts
u64 Time64Fast(void)
{
	u32 low = timer_hw->timelr; // get latched time LOW
	cb(); // compiler barrier (or else the compiler could swap the order of operations)
	u32 high = timer_hw->timehr; // get latched time HIGH
	return ((u64)high << 32) | low;
}

// get 64-bit absolute system time in [us] - atomic method (concurrently safe)
u64 Time64(void)
{
	u32 low, high;
	u32 high2 = timer_hw->timerawh;  // get raw time HIGH

	do {
		// accept new time HIGH
		high = high2;

		// get raw time LOW
		cb(); // compiler barrier (or else the compiler could swap the order of operations)
		low = timer_hw->timerawl;

		// get raw time HIGH again
		cb(); // compiler barrier (or else the compiler could swap the order of operations)
		high2 = timer_hw->timerawh;

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

// claim free alarm (returns -1 on error)
s8 AlarmClaimFree(void)
{
	int inx;
	for (inx = 0; inx < ALARMS_NUM; inx++)
	{
		// check if alarm is already claimed
		if (!AlarmClaimed[inx])
		{
			// claim this alarm
			AlarmClaimed[inx] = True;
			return inx;
		}
	}

	// no free alarm
	return -1;
}

#if USE_IRQ	// use IRQ interrupts (sdk_irq.c, sdk_irq.h)

// start alarm
//   alarm = alarm number 0..3
//   handler = interrupt handler
//   time = time interval in [us] after which to activate first alarm (min 3 us, max 71 minutes)
// Vector table must be located in RAM
// If vector table is not in RAM, use services with names isr_timer_0..isr_timer_3
// Call AlarmAck on start of interrupt, then AlarmRestart to restart again, or AlarmStop to deactivate.
// Alarm handler is shared between both processor cores. Only alarms of different numbers can be independent.
void AlarmStart(int alarm, irq_handler_t handler, u32 time)
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
	RegSet(&timer_hw->inte, BIT(alarm));

	// arm timer
	if (time < 3) time = 3;
	IRQ_LOCK;
	timer_hw->alarm[alarm] = time + Time();
	IRQ_UNLOCK;
}

// restart alarm - can be called from an interrupt for a repeated alarm
//   time = time interval in [us] after which to activate next alarm (max 71 minutes)
// The maximum achievable interrupt frequency is about 100 kHz.
void AlarmRestart(int alarm, u32 time)
{
	// re-arm timer
	IRQ_LOCK;
	time += timer_hw->alarm[alarm]; // next absolute time
	u32 t = Time(); // current time
	s32 dt = (s32)(time - t); // remaining time
	if ((dt < 4) && (dt > -50)) time = t + 4;
	timer_hw->alarm[alarm] = time;
	IRQ_UNLOCK;
}

// stop alarm - can be called from an interrupt if no next interrupt is required
void AlarmStop(int alarm)
{
	IRQ_LOCK;

	// disable interrupt from the timer
	RegClr(&timer_hw->inte, BIT(alarm));

	// disarm alarm
	timer_hw->armed = BIT(alarm);

	// acknowledge alarm request
	AlarmAck(alarm);

	// disable interrupt on NVIC
	NVIC_IRQDisable(ALARM_IRQ(alarm));

	IRQ_UNLOCK;
}

#endif // USE_IRQ

// ----------------------------------------------------------------------------
//                  System clock timer (only RP2350; uses TIMER1)
// ----------------------------------------------------------------------------

#if RP2350		// 1=use MCU RP2350

// claimed alarms
Bool Alarm2Claimed[ALARMS_NUM];	// claimed alarms

// get 64-bit absolute system time in sys_clk ticks - fast method, but it is not atomic safe
//   - do not use simultaneously from both processor cores and from interrupts
u64 TimeSysClk64Fast(void)
{
	u32 low = timer1_hw->timelr; // get latched time LOW
	cb(); // compiler barrier (or else the compiler could swap the order of operations)
	u32 high = timer1_hw->timehr; // get latched time HIGH
	return ((u64)high << 32) | low;
}

// get 64-bit absolute system time in sys_clk ticks - atomic method (concurrently safe)
u64 TimeSysClk64(void)
{
	u32 low, high;
	u32 high2 = timer1_hw->timerawh;  // get raw time HIGH

	do {
		// accept new time HIGH
		high = high2;

		// get raw time LOW
		cb(); // compiler barrier (or else the compiler could swap the order of operations)
		low = timer1_hw->timerawl;

		// get raw time HIGH again
		cb(); // compiler barrier (or else the compiler could swap the order of operations)
		high2 = timer1_hw->timerawh;

	// check that HIGH has not changed, otherwise a re-read will be necessary
	} while (high != high2);

	// return result value
	return ((u64)high << 32) | low;
}

// wait for delay in sys_clk ticks
void WaitSysClk(u32 sysclk)
{
	u32 start = TimeSysClk();
	while ((u32)(TimeSysClk() - start) < sysclk) {}
}

// claim free alarm (returns -1 on error)
s8 Alarm2ClaimFree(void)
{
	int inx;
	for (inx = 0; inx < ALARMS_NUM; inx++)
	{
		// check if alarm is already claimed
		if (!Alarm2Claimed[inx])
		{
			// claim this alarm
			Alarm2Claimed[inx] = True;
			return inx;
		}
	}

	// no free alarm
	return -1;
}

#if USE_IRQ	// use IRQ interrupts (sdk_irq.c, sdk_irq.h)

// start alarm
//   alarm = alarm number 0..3
//   handler = interrupt handler
//   time = time interval in sys_clk after which to activate first alarm (min. 50)
// Vector table must be located in RAM
// If vector table is not in RAM, use services with names isr_timer_0..isr_timer_3
// Call Alarm2Ack on start of interrupt, then Alarm2Restart to restart again, or Alarm2Stop to deactivate.
// Alarm handler is shared between both processor cores. Only alarms of different numbers can be independent.
void Alarm2Start(int alarm, irq_handler_t handler, u32 time)
{
	// stop previous alarm
	Alarm2Stop(alarm);

	// prepare interrupt IRQ
	u8 irq = ALARM2_IRQ(alarm);

	// set interrupt handler to VTOR interrupt table (VTOR should be in RAM)
	SetHandler(irq, handler);

	// enable interrupt on NVIC
	NVIC_IRQEnable(irq);

	// enable interrupt from the timer
	RegSet(&timer1_hw->inte, BIT(alarm));

	// arm timer
	if (time < 50) time = 50;
	IRQ_LOCK;
	timer1_hw->alarm[alarm] = time + TimeSysClk();
	IRQ_UNLOCK;
}

// restart alarm - can be called from an interrupt for a repeated alarm
//   time = time interval in sys_clk after which to activate next alarm
// The maximum achievable interrupt frequency is about 100 kHz.
void Alarm2Restart(int alarm, u32 time)
{
	// re-arm timer
	IRQ_LOCK;
	time += timer1_hw->alarm[alarm]; // next absolute time
	u32 t = TimeSysClk(); // current time
	s32 dt = (s32)(time - t); // remaining time
	if ((dt < 50) && (dt > -200)) time = t + 50;
	timer1_hw->alarm[alarm] = time;
	IRQ_UNLOCK;
}

// stop alarm - can be called from an interrupt if no next interrupt is required
void Alarm2Stop(int alarm)
{
	IRQ_LOCK;

	// disable interrupt from the timer
	RegClr(&timer1_hw->inte, BIT(alarm));

	// disarm alarm
	timer1_hw->armed = BIT(alarm);

	// acknowledge alarm request
	Alarm2Ack(alarm);

	// disable interrupt on NVIC
	NVIC_IRQDisable(ALARM2_IRQ(alarm));

	IRQ_UNLOCK;
}

#endif // USE_IRQ

#endif // RP2350		// 1=use MCU RP2350

// ----------------------------------------------------------------------------
//                RISC-V platform machine-mode timer (only RP2350)
// ----------------------------------------------------------------------------

#if RP2350		// 1=use MCU RP2350

// get 64-bit time from RISC-V machine-mode timer - fast method, but it is not atomic safe
//   - do not use simultaneously from both processor cores and from interrupts
u64 MTime64Fast(void)
{
	u32 low = sio_hw->mtime; // get time LOW
	cb(); // compiler barrier (or else the compiler could swap the order of operations)
	u32 high = sio_hw->mtimeh; // get time HIGH
	return ((u64)high << 32) | low;
}

// get 64-bit time from RISC-V machine-mode timer - atomic method (concurrently safe)
u64 MTime64(void)
{
	u32 low, high;
	u32 high2 = sio_hw->mtimeh;  // get time HIGH

	do {
		// accept new time HIGH
		high = high2;

		// get time LOW
		cb(); // compiler barrier (or else the compiler could swap the order of operations)
		low = sio_hw->mtime;

		// get raw time HIGH again
		cb(); // compiler barrier (or else the compiler could swap the order of operations)
		high2 = sio_hw->mtimeh;

	// check that HIGH has not changed, otherwise a re-read will be necessary
	} while (high != high2);

	// return result value
	return ((u64)high << 32) | low;
}

// set time to RISC-V machine-mode timer
void MTimeSet(u64 time)
{
	// disable interrupts (so that the operation does not take too long and the LOW timer does not overflow)
	IRQ_LOCK;

	// reset timer LOW, to avoid the possibility of overflow into the HIGH
	sio_hw->mtime  = 0;

	// set timer HIGH
	cb(); // compiler barrier (or else the compiler could swap the order of operations)
	sio_hw->mtimeh = (u32)(time >> 32);

	// set timer LOW
	cb(); // compiler barrier (or else the compiler could swap the order of operations)
	sio_hw->mtime = (u32)time;

	// restore interrupts
	IRQ_UNLOCK;
}

// get current compare value of RISC-V machine-mode timer
u64 MTimeGetCmp(void)
{
	// disable interrupts (for case the interrupt would change the value of the register)
	IRQ_LOCK;

	// get value
	u32 low = sio_hw->mtimecmp; // get timecmp LOW
	u32 high = sio_hw->mtimecmph; // get timecmp HIGH

	// restore interrupts
	IRQ_UNLOCK;

	return ((u64)high << 32) | low;
}

// set compare value of RISC-V machine-mode timer (each core has its own copy of compare registers)
//  Interrupt is asserted whenever the 64-bit mtime value is greater than or equal to compare value.
//  Raises IRQ_SIO_MTIMECMP interrupt on ARM, and mip.mtip interrupt on RISC-V.
void MTimeCmp(u64 timecmp)
{
	// disable interrupts (for case the interrupt would change the value of the register)
	IRQ_LOCK;

	// disable compare value
	sio_hw->mtimecmph = (u32)-1;

	// set new value LOW
	cb(); // compiler barrier (or else the compiler could swap the order of operations)
	sio_hw->mtimecmp = (u32)timecmp;

	// set new value HIGH
	cb(); // compiler barrier (or else the compiler could swap the order of operations)
	sio_hw->mtimecmph = (u32)(timecmp >> 32);
	
	// restore interrupts
	IRQ_UNLOCK;
}

#endif // RP2350

// ----------------------------------------------------------------------------
//                          Original-SDK interface
// ----------------------------------------------------------------------------

#if USE_ORIGSDK		// include interface of original SDK

// alarm handler
void hardware_alarm_irq_handler(void)
{
	// get alarm number
	int alarm = GetCurrentIRQ() - IRQ_TIMER_0;
	if ((u32)alarm >= ALARMS_NUM) return;

	// prepare callback user function
	hardware_alarm_callback_t callback = NULL;

	// get spinlock
	spin_lock_t* lock = SpinGetHw(PICO_SPINLOCK_ID_TIMER);

	// lock timer spinlock
	u32 save = spin_lock_blocking(lock);

	// clear timer IRQ request
	AlarmAck(alarm);

	// clear forced request
	AlarmUnforce(alarm);

	// check if IRQ is still valid
	if ((timer_callbacks_pending & BIT(alarm)) != 0)
	{
		// check timer HIGH
		if (timer_hw->timerawh >= target_hi[alarm])
		{
			// this alarm is valid, get alarm handler
			callback = alarm_callbacks[alarm];

			// clear pending flag
			timer_callbacks_pending &= (u8)~BIT(alarm);
		}

		// not reached yet, re-arm timer for later
		else
			timer_hw->alarm[alarm] = timer_hw->alarm[alarm];
	}

	// unlock timer
	spin_unlock(lock, save);

	// call user handler
	if (callback != NULL) callback(alarm);
}

// Enable/Disable a callback for a hardware timer on this core
void hardware_alarm_set_callback(uint alarm_num, hardware_alarm_callback_t callback)
{
	// IRQ number
	u8 irq = ALARM_IRQ(alarm_num);

	// get spinlock
	spin_lock_t* lock = SpinGetHw(PICO_SPINLOCK_ID_TIMER);

	// lock timer spinlock
	u32 save = spin_lock_blocking(lock);

	// setup callback
	if (callback != NULL)
	{
		// set user handler
		alarm_callbacks[alarm_num] = callback;

		// register alarm handler
		SetHandler(irq, hardware_alarm_irq_handler);

		// enable IRQ
		NVIC_IRQEnable(irq);

		// enable alarm interrupt
		RegSet(&timer_hw->inte, BIT(alarm_num));
	}

	// clear callback
	else
	{
		// disable IRQ
		NVIC_IRQDisable(irq);

		// clear user handler
		alarm_callbacks[alarm_num] = NULL;

		// clear pending flag
		timer_callbacks_pending &= (u8)~BIT(alarm_num);
	}
		
	// unlock timer
	spin_unlock(lock, save);
}

// Set the current target for the specified hardware alarm
bool hardware_alarm_set_target(uint alarm_num, absolute_time_t target)
{
	// get current time
	u64 now = Time64();

	// target time
	u64 t = to_us_since_boot(target);

	// is target time missed?
	Bool missed = now >= t;

	// not missed
	if (!missed)
	{
		// get spinlock
		spin_lock_t* lock = SpinGetHw(PICO_SPINLOCK_ID_TIMER);

		// lock timer spinlock
		u32 save = spin_lock_blocking(lock);

		// set pending flag
		u8 pending = timer_callbacks_pending;
		timer_callbacks_pending = pending | BIT(alarm_num);

		// clear timer IRQ request
		AlarmAck(alarm_num);

		// arm alarm LOW
		timer_hw->alarm[alarm_num] = (u32)t;

		// save alarm HIGH
		target_hi[alarm_num] = (u32)(t >> 32);

		// check if alarm is armed (or it has been fired already otherwise)
		if ((timer_hw->armed & BIT(alarm_num)) != 0)
		{
			// if target is missed
			if (Time64() >= t)
			{
				// missed flag
				missed = True;

				// was not pending?
				if (pending != timer_callbacks_pending)
				{
					// disarm timer
					timer_hw->armed = BIT(alarm_num);

					// clear IRQ request
					AlarmAck(alarm_num);

					// clear IRQ
					NVIC_IRQClear(ALARM_IRQ(alarm_num));

					// restore pending flags
					timer_callbacks_pending = pending;
				}
			}
		}

		// unlock timer
		spin_unlock(lock, save);
	}
	return missed;
}

// Cancel an existing target (if any) for a given hardware_alarm
void hardware_alarm_cancel(uint alarm_num)
{
	// get spinlock
	spin_lock_t* lock = SpinGetHw(PICO_SPINLOCK_ID_TIMER);

	// lock timer spinlock
	u32 save = spin_lock_blocking(lock);

	// disarm timer
	timer_hw->armed = BIT(alarm_num);

	// clear pending flag
	timer_callbacks_pending &= (u8)~BIT(alarm_num);

	// unlock timer
	spin_unlock(lock, save);
}

// Force and IRQ for a specific hardware alarm
void hardware_alarm_force_irq(uint alarm_num)
{
	// get spinlock
	spin_lock_t* lock = SpinGetHw(PICO_SPINLOCK_ID_TIMER);

	// lock timer spinlock
	u32 save = spin_lock_blocking(lock);

	// set pending flag
	timer_callbacks_pending |= BIT(alarm_num);

	// unlock timer
	spin_unlock(lock, save);

	// force alarm
	AlarmForce(alarm_num);
}

// === alarm pool, timers

// alarm pool handler
void alarm_pool_irq_handler(void)
{
	// get alarm number
	int alarm = GetCurrentIRQ() - IRQ_TIMER_0;
	if ((u32)alarm >= ALARMS_NUM) return;

	// clear timer IRQ request
	AlarmAck(alarm);

	// clear forced request
	AlarmUnforce(alarm);

	// get alarm handler
	alarm_callback_t callback = timer_callbacks[alarm];
	void* data = timer_data[alarm];

	// call user handler
	if (callback != NULL)
	{
		// call callback
		s64 d = callback(alarm, data);

		// not reschedule
		if (d == 0)
		{
			// disable alarm
			RegClr(&timer_hw->inte, BIT(alarm));
			timer_hw->armed = BIT(alarm);
			NVIC_IRQDisable(ALARM_IRQ(alarm));
			AlarmUnclaim(alarm);
		}
		else
		{
			// IRQ lock
			IRQ_LOCK;

			// prepare next time to reschedule
			u32 next;
			if (d > 0) // reschedule from current time
				next = Time() + (u32)d;
			else // reschedule from previously scheduled time
				next = timer_hw->alarm[alarm] + (u32)(-d);

			// check elapsed time
			u32 t = Time();
			s32 dt = (s32)(next - t);
			if ((dt < 4) && (dt > -50)) next = t + 4;

			// rearm alarm
			timer_hw->alarm[alarm] = next;

			// IRQ unlock
			IRQ_UNLOCK;
		}
	}
	else
	{
		// disable alarm
		RegClr(&timer_hw->inte, BIT(alarm));
		timer_hw->armed = BIT(alarm);
		NVIC_IRQDisable(ALARM_IRQ(alarm));
		AlarmUnclaim(alarm);
	}
}

// Add an alarm callback to be called at a specific time
alarm_id_t alarm_pool_add_alarm_at(alarm_pool_t *pool, absolute_time_t time, alarm_callback_t callback, void *user_data, bool fire_if_past)
{
	if (callback == NULL) return -1;

	// get free timer
	s32 alarm = AlarmClaimFree();
	if (alarm < 0) return alarm;

	// IRQ number
	u8 irq = ALARM_IRQ(alarm);

	// IRQ lock
	IRQ_LOCK;

	// register callback
	timer_callbacks[alarm] = callback;
	timer_data[alarm] = user_data;

	// register alarm handler
	SetHandler(irq, alarm_pool_irq_handler);

	// enable IRQ
	NVIC_IRQEnable(irq);

	// enable alarm interrupt
	RegSet(&timer_hw->inte, BIT(alarm));

	// clear timer IRQ request
	AlarmAck(alarm);

	// prepare time
	u32 next = (u32)time;
	u32 t = Time();
	s32 dt = (s32)(next - t);
	if ((dt < 4) && (dt > -50)) next = t + 4;

	// arm alarm
	timer_hw->alarm[alarm] = next;

	// IRQ unlock
	IRQ_UNLOCK;

	return (alarm_id_t)alarm;
}

s64 repeating_timer_callback(__unused alarm_id_t id, void *user_data)
{
	repeating_timer_t *rt = (repeating_timer_t*)user_data;
	if (rt->callback(rt)) return rt->delay_us;
	rt->alarm_id = -1;
	return 0;
}

// Cancel an alarm
bool alarm_pool_cancel_alarm(alarm_pool_t *pool, alarm_id_t alarm_id)
{
	s32 alarm = (s32)alarm_id;
	if ((u32)alarm >= ALARMS_NUM) return false;
	IRQ_LOCK;
	RegClr(&timer_hw->inte, BIT(alarm));
	timer_hw->armed = BIT(alarm);
	NVIC_IRQDisable(ALARM_IRQ(alarm));
	AlarmUnclaim(alarm);
	IRQ_UNLOCK;
	return true;
}

// Add a repeating timer that is called repeatedly at the specified interval in microseconds
bool alarm_pool_add_repeating_timer_us(alarm_pool_t *pool, s64 delay_us, repeating_timer_callback_t callback, void *user_data, repeating_timer_t *out)
{
	if (delay_us == 0) delay_us = 1;
	out->pool = pool;
	out->callback = callback;
	out->delay_us = delay_us;
	out->user_data = user_data;
	out->alarm_id = alarm_pool_add_alarm_at(pool, make_timeout_time_us((u64)(delay_us >= 0 ? delay_us : -delay_us)), repeating_timer_callback, out, true);
	return out->alarm_id >= 0;
}

// Cancel a repeating timer
bool cancel_repeating_timer(repeating_timer_t *timer)
{
	bool rc = alarm_pool_cancel_alarm(timer->pool, timer->alarm_id);
	timer->alarm_id = -1;
	return rc;
}

#endif // USE_ORIGSDK

#endif // USE_TIMER	// use Timer with alarm (sdk_timer.c, sdk_timer.h)
