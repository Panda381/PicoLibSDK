
// ****************************************************************************
//
//                              Watchdog Timer
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

// Time source is shared with main timer counter.

#ifndef _SDK_WATCHDOG_H
#define _SDK_WATCHDOG_H

#include "../sdk_addressmap.h"		// Register address offsets
#include "sdk_reset.h"			// register PSM_WDSEL

#if USE_ORIGSDK		// include interface of original SDK
#include "orig/orig_watchdog.h"	// constants of original SDK
#endif // USE_ORIGSDK

#ifdef __cplusplus
extern "C" {
#endif

// Watchdog hardware registers
//#define WATCHDOG_BASE		0x40058000	// Watchdog timer
#define WATCHDOG_CTRL	((volatile u32*)(WATCHDOG_BASE + 0x00)) // control register
#define WATCHDOG_LOAD	((volatile u32*)(WATCHDOG_BASE + 0x04)) // load
#define WATCHDOG_REASON	((volatile u32*)(WATCHDOG_BASE + 0x08)) // reason
#define WATCHDOG_SCRATCH ((volatile u32*)(WATCHDOG_BASE + 0x0C)) // 8 scratch registers u32
#define WATCHDOG_TICK	((volatile u32*)(WATCHDOG_BASE + 0x2C)) // tick

// Watchdog hardware interface
typedef struct {
	io32	ctrl;		// 0x00: Watchdog control
	io32	load;		// 0x04: Load the watchdog timer
	io32	reason;		// 0x08: Logs the reason for the last reset
	io32	scratch[8];	// 0x0C: Scratch register
	io32	tick;		// 0x2C: Controls the tick generator
} watchdog_hw_t;

#define watchdog_hw ((watchdog_hw_t*)WATCHDOG_BASE)

STATIC_ASSERT(sizeof(watchdog_hw_t) == 0x30, "Incorrect watchdog_hw_t!");

// Control register WATCHDOG_CTRL
#define WATCHDOG_CTRL_TIMEMASK	0x00ffffff		// number of ticks*2
#define WATCHDOG_REBOOT_MAGIC	0xB007C0D3		// magic of soft reset (for using in scratch registers)
#define WATCHDOG_LOADER_MAGIC	0x64616F4C		// magic of boot loader (= text "Load")
#define WATCHDOG_NON_REBOOT_MAGIC 0x6ab73121		// watchdog reset by WatchdogEnable()

#if USE_WATCHDOG || USE_TIMER // use Watchdog or Timer
// Start watchdog tick generator (generated ticks are shared with main timer counter!)
void WatchdogStart(u32 xosc_mhz);
#endif // USE_WATCHDOG || USE_TIMER // use Watchdog or Timer

#if USE_WATCHDOG	// use Watchdog timer (sdk_watchdog.c, sdk_watchdog.h)

// Watchdog soft reboot
//   scratch register 4: magic number 0xb007c0d3 (will be cleared before transferring control)
//   scratch register 5: entry point XORed with magic -0xb007c0d3 (0x4ff83f2d)
//   scratch register 6: stack pointer
//   scratch register 7: entry point

// watchdog update value (to restart time counting) = time in [us] * 2
extern u32 WatchdogUpdateValue;

// update watchdog (restart time counting)
INLINE void WatchdogUpdate(void) { watchdog_hw->load = WatchdogUpdateValue; }

// Get current value of watchdog generator in number of us
INLINE u32 WatchdogGetCount(void) { return (watchdog_hw->ctrl & WATCHDOG_CTRL_TIMEMASK) / 2; }

// Setup watchdog generator (does not set reason into scratch registers)
//  us = delay before reset in [us], max. 0x7fffff us (8388607 us = 8.4 seconds)
//  pause_debug = pause watchdog on debugging
void WatchdogSetup(u32 us, Bool pause_debug);

// watchdog enable (set reason WATCHDOG_NON_REBOOT_MAGIC)
//  us = delay before reset in [us], max. 0x7fffff us (8388607 us = 8.4 seconds)
//  pause_debug = pause watchdog on debugging
void WatchdogEnable(u32 us, Bool pause_debug);

// Setup watchdog generator to do hard reset (set reason 0)
//  us = delay before reset in [us], max. 0x7fffff us (8388607 us = 8.4 seconds)
//  pause_debug = pause watchdog on debugging
void WatchdogSetupReset(u32 us, Bool pause_debug);

// Setup watchdog genererator to do soft reset
//  us = delay before reset in [us], max. 0x7fffff us (8388607 us = 8.4 seconds)
//  pc = pointer to code to restart in soft reset (0 = do hard reset)
//  sp = stack pointer
void WatchdogSetupReboot(u32 us, u32 pc, u32 sp);

// Trigger watchdog reset
INLINE void WatchdogTrigger(void) { RegSet(&watchdog_hw->ctrl, B31); }

// Get reset reason (0=hardware, 1=watchdog timer, 2=force trigger)
INLINE u8 WatchdogReason(void) { return (u8)(watchdog_hw->reason & (B0|B1)); }

// Check reset reason WatchdogEnable()
Bool WatchdogReasonEnable(void);

// reset to boot loader
void ResetToBootLoader(void);

// ----------------------------------------------------------------------------
//                          Original-SDK interface
// ----------------------------------------------------------------------------

#if USE_ORIGSDK		// include interface of original SDK

// Define actions to perform at watchdog timeout
INLINE void watchdog_reboot(u32 pc, u32 sp, u32 delay_ms) { WatchdogSetupReboot(delay_ms*1000, pc, sp); }

// Start the watchdog tick
INLINE void watchdog_start_tick(uint cycles) { WatchdogStart(cycles); }

// Reload the watchdog counter with the amount of time set in watchdog_enable
INLINE void watchdog_update(void) { WatchdogUpdate(); }

// Enable the watchdog
INLINE void watchdog_enable(u32 delay_ms, bool pause_on_debug) { WatchdogEnable(delay_ms*1000, pause_on_debug); }

// Did the watchdog cause the last reboot?
INLINE bool watchdog_caused_reboot(void) { return WatchdogReason(); }

// Did watchdog_enable cause the last reboot?
INLINE bool watchdog_enable_caused_reboot(void) { return WatchdogReasonEnable(); }

// Returns the number of microseconds before the watchdog will reboot the chip.
INLINE u32 watchdog_get_count(void) { return WatchdogGetCount(); }

#endif // USE_ORIGSDK

#endif // USE_WATCHDOG	// use Watchdog timer (sdk_watchdog.c, sdk_watchdog.h)

#ifdef __cplusplus
}
#endif

#endif // _SDK_WATCHDOG_H
