
// ****************************************************************************
//
//                            ROSC ring oscillator
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

// Ring Oscillator (ROSC) is an on-chip oscillator built from a ring of inverters.
// During boot the ROSC runs at a nominal 6.5MHz (with tolerance 1.8MHz to 12MHz).

#if USE_ROSC	// use ROSC ring oscillator (sdk_rosc.c, sdk_rosc.h)

#ifndef _SDK_ROSC_H
#define _SDK_ROSC_H

#include "../sdk_addressmap.h"		// Register address offsets
#include "sdk_clocks.h"			// clocks

#if USE_ORIGSDK		// include interface of original SDK
#if RP2040		// 1=use MCU RP2040
#include "orig_rp2040/orig_rosc.h"	// constants of original SDK
#else
#include "orig_rp2350/orig_rosc.h"	// constants of original SDK
#endif
#endif // USE_ORIGSDK

#ifdef __cplusplus
extern "C" {
#endif

// ROSC hardware registers
#define ROSC_CTRL	((volatile u32*)(ROSC_BASE+0x00)) // ROSC control
#define ROSC_FREQA	((volatile u32*)(ROSC_BASE+0x04)) // ROSC frequency control A
#define ROSC_FREQB	((volatile u32*)(ROSC_BASE+0x08)) // ROSC frequency control B

#if RP2040
#define ROSC_DORMANT	((volatile u32*)(ROSC_BASE+0x0C)) // ROSC pause control
#define ROSC_DIV	((volatile u32*)(ROSC_BASE+0x10)) // ROSC output divider
#define ROSC_PHASE	((volatile u32*)(ROSC_BASE+0x14)) // ROSC phase shifted output
#define ROSC_STATUS	((volatile u32*)(ROSC_BASE+0x18)) // ROSC status
#define ROSC_RANDBIT	((volatile u32*)(ROSC_BASE+0x1C)) // ROSC random bit
#define ROSC_COUNT	((volatile u32*)(ROSC_BASE+0x20)) // ROSC down counter
#else
#define ROSC_RANDOM	((volatile u32*)(ROSC_BASE+0x0C)) // ROSC randomiser
#define ROSC_DORMANT	((volatile u32*)(ROSC_BASE+0x10)) // ROSC pause control
#define ROSC_DIV	((volatile u32*)(ROSC_BASE+0x14)) // ROSC output divider
#define ROSC_PHASE	((volatile u32*)(ROSC_BASE+0x18)) // ROSC phase shifted output
#define ROSC_STATUS	((volatile u32*)(ROSC_BASE+0x1C)) // ROSC status
#define ROSC_RANDBIT	((volatile u32*)(ROSC_BASE+0x20)) // ROSC random bit
#define ROSC_COUNT	((volatile u32*)(ROSC_BASE+0x24)) // ROSC down counter
#endif

#define ROSC_FREQ_NUM	64	// number of ROSC frequency levels

// ROSC hardware interface
typedef struct {
	io32	ctrl;		// 0x00: Ring Oscillator control
	io32	freqa;		// 0x04: The FREQA & FREQB registers control the frequency by controlling the drive strength of each stage
	io32	freqb;		// 0x08: For a detailed description see freqa register
#if !RP2040
	io32	random;		// 0x0C: value from LFSR randomiser
#endif
	io32	dormant;	// 0x0C (0x10): Ring Oscillator pause control
	io32	div;		// 0x10 (0x14): Controls the output divider
	io32	phase;		// 0x14 (0x18): Controls the phase shifted output
	io32	status;		// 0x18 (0x1C): Ring Oscillator Status
	io32	randombit;	// 0x1C (0x20): This just reads the state of the oscillator output so randomness is compromised if the ring oscillator is stopped or...
	io32	count;		// 0x20 (0x24): A down counter running at the ROSC frequency which counts to zero and stops
} rosc_hw_t;

#define rosc_hw ((rosc_hw_t*)ROSC_BASE)

#if RP2040
STATIC_ASSERT(sizeof(rosc_hw_t) == 0x24, "Incorrect rosc_hw_t!");
#else
STATIC_ASSERT(sizeof(rosc_hw_t) == 0x28, "Incorrect rosc_hw_t!");
#endif

// Enable ring oscillator
INLINE void RoscEnable(void) { RegMask(&rosc_hw->ctrl, 0xFAB << 12, 0xFFF << 12); }

// Disable ring oscillator (cannot be disabled, if CPU uses it as a clock source)
INLINE void RoscDisable(void) { RegMask(&rosc_hw->ctrl, 0xD1E << 12, 0xFFF << 12); }

#if !RP2040
// Enable frequency randomisation (only RP2350)
INLINE void RoscRandEnable(void) { rosc_hw->freqa = (rosc_hw->freqa & 0x7777) | 0x96960000 | (B3+B7); }

// Disable frequency randomisation (only RP2350)
INLINE void RoscRandDisable(void) { rosc_hw->freqa = (rosc_hw->freqa & 0x7777) | 0x96960000; }
#endif

// Current ROSC frequency level set by the RoscSetLevel function
extern u8 RoscLevel;

// set ring oscillator frequency level (level is in range 0..63, 0..ROSC_FREQ_NUM-1; 0 is default level)
//  Typical frequency ranges:
//    level 0..24, low frequency range (8 stages), 81..150 MHz
//    level 25..43, medium frequency range (6 stages), 101..181 MHz
//    level 44..56, high frequency range (4 stages), 136..232 MHz
//    level 57..63, too high frequency range (2 stages), 217..321 MHz
// Currently set frequency level can be read from the RoscLevel variable.
void RoscSetLevel(int level);

// set seed of ROSC randomiser (only RP2350; default value ROSC_SEED_DEF)
#if !RP2040
#define ROSC_SEED_DEF	0x3F04B16D
INLINE void RoscSetSeed(u32 seed) { rosc_hw->random = seed; }
#endif

// set ROSC divider 1..32 (RP2040) or 1..128 (RP2350) (default value RP2040: 16, RP2350: 8)
//  RP2040: Default frequency level 0 and default divider value 16 give typical default frequency 81/16 = 5 MHz
//  RP2350: Default frequency level 0 and default divider value 8 give typical default frequency 81/8 = 10 MHz
#if RP2040
INLINE void RoscSetDiv(int div) { rosc_hw->div = 0xAA0 + (div & 0x1F); }
#else
INLINE void RoscSetDiv(int div) { rosc_hw->div = 0xAA00 + (div & 0x7F); }
#endif

// get ROSC divider 1..32 (RP2040) or 1..128 (RP2350)
int RoscGetDiv(void);

// initialize ROSC ring oscillator to its default state (typical frequency RP2040: 6 MHz, RP2350: 11 MHz)
void RoscInit(void);

// select ring oscillator frequency in range 2.5 .. 255 MHz (input value freq10 means required frequency in MHz * 10)
//  Selected frequency is very approximate, it can vary from 50% to 200%. 
//  The main usage of the function is in conjunction with the XOSC oscillator
//    so that frequency can be continuously changed during calibration.
void RoscSetFreq(int freq10);

// start dormant mode of the ring oscillator (text "coma")
INLINE void RoscDormant(void) { rosc_hw->dormant = 0x636f6d61; }

// wake up ring oscillator from dormant mode (text "wake")
INLINE void RoscWake(void) { rosc_hw->dormant = 0x77616b65; }

// check if ring oscillator is running and stable
INLINE Bool RoscIsStable(void) { return (rosc_hw->status & B31) != 0; }

// wait for stable state of the ring oscillator (ROSC must be enabled)
//  Usually not needed, ring oscillator stabilizes almost immediately.
INLINE void RoscWait(void) { while (!RoscIsStable()) {} }

// get random bit from ring oscillator (low randomness)
INLINE int RoscRandBit(void) { return rosc_hw->randombit & 1; }

// get true random data from the ring oscillator combined with LCG generator
//  Takes 26 us at 125 MHz, that means speed 1.2 Mbits/sec.
//  The function is not protected against concurrency with IRQ interrupts and
//  with the second processor core. But this is not problem - concurrency will
//  overwrite the seed, but generated data will remain sufficiently randomised.
u32 RoscRand(void);
INLINE u32 RoscRand32(void) { return RoscRand(); }

// set counter of ring oscillator
INLINE void RoscSetCount(u8 count) { rosc_hw->count = count; }

// get counter of ring oscillator (after setting the value, it runs down to zero and stops there)
INLINE u8 RoscGetCount(void) { return (u8)rosc_hw->count; }

#ifdef __cplusplus
}
#endif

#endif // _SDK_ROSC_H

#endif // USE_ROSC	// use ROSC ring oscillator (sdk_rosc.c, sdk_rosc.h)
