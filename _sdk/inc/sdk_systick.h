
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

// SysTick is 24-bit counter of ARMv6-M CPU core. Each core has its own systick timer.
// SysTick in RP2040 uses 1 us ticks as clock source.

#if USE_SYSTICK		// use SysTick system timer (sdk_systick.c, sdk_systick.h)

#ifndef _SDK_SYSTICK_H
#define _SDK_SYSTICK_H

#include "../sdk_addressmap.h"		// Register address offsets

#if USE_ORIGSDK		// include interface of original SDK
#if RP2040		// 1=use MCU RP2040
#include "orig_rp2040/orig_m0plus.h"		// constants of original SDK
#else
#include "orig_rp2350/orig_m33.h"		// constants of original SDK
#endif
#endif // USE_ORIGSDK

#ifdef __cplusplus
extern "C" {
#endif

// increment of system time in [ms] on SysTick interrupt
#ifndef SYSTICK_MS
#define SYSTICK_MS	5
#endif

// SysTick period (number of 1 us ticks to interrupt every SYSTICK_MS ms)
#define SYSTICK_TICKS	(SYSTICK_MS*1000)

// system time counter, counts time from system start - incremented every SYSTICK_MS ms
// with overflow after 49 days (use difference, not absolute value!)
extern volatile u32 SysTime;

// current date and time (incremented every SYSTICK_MS ms from CPU core 0)
extern volatile u32 UnixTime;		// current date and time in Unix format
					// - number of seconds since 1/1/1970 (thursday) up to 12/31/2099
extern volatile s16 CurTimeMs;		// current time in [ms] 0..999

#if !RISCV

// === System control block (RP2040 datasheet page 74)
//#define PPB_BASE		0xe0000000	// Cortex-M0+ internal registers (system control block)

#define SYSTICK_CSR ((volatile u32*)(PPB_BASE + 0xE010))	// SysTick control and status register
#define SYSTICK_RVR ((volatile u32*)(PPB_BASE + 0xE014))	// SysTick reload value register
#define SYSTICK_CVR ((volatile u32*)(PPB_BASE + 0xE018))	// SysTick current value register
#define SYSTICK_CALIB ((volatile u32*)(PPB_BASE + 0xE01C))	// SysTick calibration value register

typedef struct {
	io32	csr;	// 0x00: Use the SysTick Control and Status Register to enable the SysTick features
	io32	rvr;	// 0x04: Use the SysTick Reload Value Register to specify the start value to load into the current value register when the...
	io32	cvr;	// 0x08: Use the SysTick Current Value Register to find the current value in the register
	io32	calib;	// 0x0C: Use the SysTick Calibration Value Register to enable software to scale to any required speed using divide and multiply
} systick_hw_t;

STATIC_ASSERT(sizeof(systick_hw_t) == 0x10, "Incorrect systick_hw_t!");

#define systick_hw ((systick_hw_t*)(PPB_BASE + 0xE010))

#if !RP2040
#define systick_ns_hw ((systick_hw_t *)(PPB_NONSEC_BASE + 0xE010))
#endif

#endif // !RISCV

// initialize SysTick timer for this CPU core to interrupt every SYSTICK_MS ms
// - AlarmInit() function must be called before initializing SysTick timers.
// - It must be called from each CPU core separately.
void SysTickInit();

// terminate SysTick timer
void SysTickTerm();

// get current date and time in Unix format with [ms] and [us], synchronized (NULL = entries not required)
//  Takes 1 us
u32 GetUnixTime(s16* ms, s16* us);

// set current date and time in Unix format with [ms] (0..999)
// - Should be called from CPU core 0. From core 1, the setting may be inaccurate.
//  Takes 280 ns
void SetUnixTime(u32 time, s16 ms);

#ifdef __cplusplus
}
#endif

#endif // _SDK_SYSTICK_H

#endif // USE_SYSTICK		// use SysTick system timer (sdk_systick.c, sdk_systick.h)
