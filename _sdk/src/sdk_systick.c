
// ****************************************************************************
//
//                                   SysTick
//                             SysTick system timer
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

#if USE_SYSTICK		// use SysTick system timer (sdk_systick.c, sdk_systick.h)

#include "../inc/sdk_cpu.h"
#include "../inc/sdk_systick.h"
#include "../inc/sdk_timer.h"
#include "../inc/sdk_irq.h"
#include "../../_lib/inc/lib_alarm.h"
#include "../usb_inc/sdk_usb_phy.h"
#include "../usb_inc/sdk_usb_host.h"

// system time counter, counts time from system start - incremented every SYSTICK_MS ms
// with overflow after 49 days (use difference, not absolute value!)
volatile u32 SysTime;

// current date and time (incremented every SYSTICK_MS ms from CPU core 0)
volatile u32 UnixTime;		// current date and time in Unix format
				// - number of seconds since 1/1/1970 (thursday) up to 12/31/2099
volatile s16 CurTimeMs;		// current time in [ms] 0..999
volatile u16 LastTimeUs;	// last [us] time during timer interrupt

#if SYSTICK_KEYSCAN	// call KeyScan() function from SysTick system timer
void KeyScan();
#endif

// SysTick handler
void SysTick_Handler()
{
#if RISCV
	// set next time tick
	MTimeCmp(MTimeGetCmp() + SYSTICK_TICKS);
#endif // RISCV

	// disable interrupts (to avoid interrupt by higher IRQ)
	IRQ_LOCK;

	// get current core
	int core = CpuID();

	// update system time - only for CPU core 0
	if (core == 0)
	{
		// increase current system time
		u32 systime = SysTime;	// get current system time
		systime += SYSTICK_MS;	// increase current system time
		SysTime = systime;	// update new value

		// save last time
#if USE_TIMER
		LastTimeUs = (u16)Time(); // get current time in [us]
#else
		LastTimeUs = 0;
#endif

		// update current date and time
		u16 ms = CurTimeMs + SYSTICK_MS; // new [ms] time
		if (ms >= 1000) // overflow to second?
		{
			ms -= 1000;
			UnixTime++; // increase Unix time
		}
		CurTimeMs = ms;	// save new [ms] time
	}

#if USE_ALARM			// use SysTick alarm (lib_alarm.c, lib_alarm.h)
	// update alarms of this CPU core
	AlarmUpdate();
#endif

	// enable interrupts
	IRQ_UNLOCK;

#if USE_USB		// use USB (sdk_usb_*.c, sdk_usb_*.h)
#if USE_USB_HOST		// use USB Host Class Driver (sdk_usb_host.c, sdk_usb_host.h; value = number of devices without HUB devices)
	// handle USB host timer - called from SysTick_Handler every 5 ms
	if (UsbHostIsInit) UsbHostOnTime();
#endif
#endif

#if SYSTICK_KEYSCAN	// call KeyScan() function from SysTick system timer
	KeyScan();
#endif

	// request to reschedule tasks (use in multitask system)
	//NVIC_PendSVForce();
}

// initialize SysTick timer for this CPU core to interrupt every SYSTICK_MS ms
// - AlarmInit() function must be called before initializing SysTick timers.
// - It must be called from each CPU core separately.
void SysTickInit()
{
	// get current core
	int core = CpuID();

	// initialize variables - only for CPU core 0
	if (core == 0)
	{
		SysTime = 0;	// reset system time counter
		UnixTime = 0;	// reset current date and time in Unix format
		CurTimeMs = 0;	// reset current time in [ms]
#if USE_TIMER
		LastTimeUs = Time(); // reset last [us] time during timer interrupt
#else
		LastTimeUs = 0;
#endif
	}

#if RISCV

	// temporary disable machine-mode timer
	MTimerDisable();

	// register machine-mode timer handler
	SetHandler(IRQ_MTIMER, SysTick_Handler);

	// initialize machine-mode timer to 5 ms
	MTimerSlow();			// set slow mode - run from 1-us tick
	u64 time = MTime64();		// get current time
	cb();
	MTimeCmp(time + SYSTICK_TICKS);	// set time of next tick
	MTimerEnable();			// enable machine-mode timer

	// enable mtimer interrupt and external interrupts
	u32 val = B7 | B11;
	__asm volatile (" csrw mie,%0\n" :: "r" (val) : "memory");

#else // RISCV

	// temporary disable SysTick counter
	systick_hw->csr = B1;	// disable SysTick handler

	// register SysTick handler
	SetHandler(IRQ_SYSTICK, SysTick_Handler);

	// initialize system tick timer to 5 ms
	systick_hw->rvr = SYSTICK_TICKS - 1; // set reload register 
	NVIC_SysTickPrio(IRQ_PRIO_SYSTICK); // set interrupt priority
	systick_hw->cvr = 0;	// reset counter
	systick_hw->csr = B0 | B1;	// set control register - counter enabled,
				//   trigger SysTick exception, use external reference clock
#endif // RISCV
}

// terminate SysTick timer
void SysTickTerm()
{
#if RISCV
	// disable mtimer interrupt and external interrupts
	u32 val = 0; //B11;
	__asm volatile (" csrw mie,%0\n" :: "r" (val) : "memory");

	// temporary disable machine-mode timer
	MTimerDisable();
#else
	// disable SysTick handler
	systick_hw->csr = B1;
#endif
}

// get current date and time in Unix format with [ms] and [us], synchronized (NULL = entries not required)
//  Takes 1 us
u32 GetUnixTime(s16* ms, s16* us)
{
	s16 m, m2, u, u2;
	u32 t, t2;
	int core;

	// get current core
	core = CpuID();

	// fast prepare old values on core 1
	if (core != 0)
	{
		// get current time in [ms]
		m2 = CurTimeMs;

		// get current time in [us]
#if USE_TIMER
		u2 = (s16)(Time() - LastTimeUs);
#else
		u2 = 0;
#endif

		// get Unix time
		t2 = UnixTime;
	}
	else
	{
		// initialization of variables to satisfy the compiler
		m2 = 0;
		u2 = 0;
		t2 = 0;
	}

	// on core 1 use multiply passes (result may be unstable)
	while (True)
	{
		// disable interrupts
		IRQ_LOCK;

		// get current time in [ms]
		m = CurTimeMs;

		// get current time in [us]
#if USE_TIMER
		u = (s16)(Time() - LastTimeUs);
#else
		u = 0;
#endif

		// get Unix time
		t = UnixTime;

		// enable interrupts
		IRQ_UNLOCK;

		// unserved pending interrupt or normalize long period
		while (u >= 1000)
		{
			u -= 1000; // [us] correction
			m++; // [ms] correction

			// [ms] overflow
			if (m >= 1000)
			{
				m -= 1000; // [ms] correction
				t++; // seconds correction
			}
		}


		// on core 0 result is valid immediately
		if (core == 0) break;

		// check difference from previous value
		u2 = u - u2;
		m2 = m - m2;
		t2 = t - t2;

		while (u2 < 0)
		{
			u2 += 1000;
			m2--;
		}

		while (u2 >= 1000)
		{
			u2 -= 1000;
			m2++;
		}

		while (m2 < 0)
		{
			m2 += 1000;
			t2--;
		}

		while (m2 >= 1000)
		{
			m2 -= 1000;
			t2++;
		}

		// check difference - must be less than 1 ms
		if ((t2 == 0) && (m2 == 0)) break;

		// save old values
		m2 = m;
		u2 = u;
		t2 = t;
	}

	// save current [us]
	if (us != NULL) *us = u;
	
	// save current [ms]
	if (ms != NULL) *ms = m;

	return t;
}

// set current date and time in Unix format with [ms] (0..999)
// - Should be called from CPU core 0. From core 1, the setting may be inaccurate.
//  Takes 280 ns
void SetUnixTime(u32 time, s16 ms)
{
	// disable interrupts
	IRQ_LOCK;

	// store new values
	UnixTime = time;
	CurTimeMs = ms;

	// enable interrupts
	IRQ_UNLOCK;
}

#endif // USE_SYSTICK		// use SysTick system timer (sdk_systick.c, sdk_systick.h)
