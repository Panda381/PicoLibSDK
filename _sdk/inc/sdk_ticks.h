
// ****************************************************************************
//
//                            Tick generator (only RP2350)
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

// Time source is CLK_REF

#if !RP2040

#ifndef _SDK_TICKS_H
#define _SDK_TICKS_H

#include "../sdk_addressmap.h"		// Register address offsets

#ifdef __cplusplus
extern "C" {
#endif

// Tick generator index (all generators should be configured to 1 us)
#define TICK_PROC0	0	// clock source of SysTick of CPU core 0
#define TICK_PROC1	1	// clock source of SysTick of CPU core 1 (should be 1 us)
#define TICK_TIMER0	2	// clock source of Timer0
#define TICK_TIMER1	3	// clock source of Timer1
#define TICK_WATCHDOG	4	// clock source of WatchDog
#define TICK_RISCV	5	// clock source for RISC-V core

#define TICK_COUNT	6

// Tick hardware interface
typedef struct {
	io32	ctrl;		// 0x00: control register
	io32	cycles;		// 0x04: number of cycles before the next tick
	io32	count;		// 0x08: count down timer
} ticks_slice_hw_t;

STATIC_ASSERT(sizeof(ticks_slice_hw_t) == 0x0C, "Incorrect ticks_slice_hw_t!");

typedef struct {
	ticks_slice_hw_t ticks[TICK_COUNT]; // 0x00: tick slices
} ticks_hw_t;

#define ticks_hw ((ticks_hw_t *)TICKS_BASE)

STATIC_ASSERT(sizeof(ticks_hw_t) == 0x48, "Incorrect ticks_hw_t!");

// start one tick generator
//  tick ... index of tick generator TICK_*
//  cycles ... clock division (source is CLK_REF)
void TickStart(int tick, u32 cycles);

// stop one tick generator
INLINE void TickStop(int tick) { ticks_hw->ticks[tick].ctrl = 0; }

// check if tick generator is running
INLINE Bool TickIsRunning(int tick) { return ticks_hw->ticks[tick].ctrl & B1; };

// start all tick generators (cycles = frequency of CLK_REF in MHz)
void TickStartAll(u32 cycles);

#ifdef __cplusplus
}
#endif

#endif // _SDK_TICKS_H

#endif // !RP2040
