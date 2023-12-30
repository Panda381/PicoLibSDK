
// ****************************************************************************
//
//                                 Emulators
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

#if USE_EMU			// use emulators

#ifndef _EMU_H
#define _EMU_H

#ifdef __cplusplus
extern "C" {
#endif

#define EMU_DEBUG_SYNC	0	// 1 = debug measure time synchronization

#if EMU_DEBUG_SYNC	// 1 = debug measure time synchronization
extern volatile Bool EmuDebClockRes;	// request to reset measure
extern u32 EmuDebClockOld;		// old clock counter
extern volatile u32 EmuDebClockTotal;	// sum of total clock intervals
extern volatile u32 EmuDebClockUsed;	// sum of used clock intervals
extern volatile s32 EmuDebClockMax;	// max used clock interval
extern volatile u32 EmuDebClockMaxTot;	// total clock interval on max
#endif // EMU_DEBUG_SYNC

// time synchronization (8 bytes)
// - Keep compatibility of the structure with the #define version in machine code!
// - Entries must be aligned
typedef struct {
	u32		clock;		// 0x00: clock counter
	volatile u32*	timer;		// 0x04: pointer to timer
} sEmuSync;

// initialize time synchronization (initialize PWM counter; returns real frequency in Hz)
//  pwm ... index of (unused) PWM slice (0..7)
//  freq ... required frequency in Hz (for 125 MHz system clock supported range: 488 kHz to 125 MHz)
u32 EmuSyncInit(sEmuSync* s, int pwm, u32 freq);

// terminate time synchronization (stop PWM counter)
//  pwm ... index of used PWM slice (0..7)
void EmuSyncTerm(int pwm);

// start time synchronization (= sets "time 0"; should be called before first program execution)
INLINE void EmuSyncStart(sEmuSync* s)
{
	s->clock = *s->timer;
#if EMU_DEBUG_SYNC	// 1 = debug measure time synchronization
	EmuDebClockRes = True; // request to reset measure
#endif
}

// time synchronization
INLINE void EmuSync(sEmuSync* s)
{
	u32 c = s->clock;
	volatile u32* t = s->timer;

#if EMU_DEBUG_SYNC	// 1 = debug measure time synchronization

	// reset measure
	if (EmuDebClockRes)
	{
		EmuDebClockMax = 0;
		EmuDebClockRes = False;
	}
	else
	{
		// total
		s16 total = (s16)(c - EmuDebClockOld);
		EmuDebClockTotal += total;
		s16 used = (s16)(*t - EmuDebClockOld);
		EmuDebClockUsed += used;

		// max
		if (used > EmuDebClockMax)
		{
			EmuDebClockMax = used;
			EmuDebClockMaxTot = total;
		}
	}

	// new start interval
	EmuDebClockOld = c;

#endif // EMU_DEBUG_SYNC

	while ((s16)(c - *t) > 0) {}
}

// function to read byte from memory or port (8-bit address)
typedef u8 (*pEmu8Read8)(u8 addr);

// function to read byte from memory or port (16-bit address)
typedef u8 (*pEmu16Read8)(u16 addr);

// function to read word from memory or port
//typedef u16 (*pEmu16Read16)(u16 addr);

// function to read dword from memory or port
//typedef u32 (*pEmu16Read32)(u16 addr);

// function to write byte to memory or port (8-bit address)
typedef void (*pEmu8Write8)(u8 addr, u8 data);

// function to write byte to memory or port (16-bit address)
typedef void (*pEmu16Write8)(u16 addr, u8 data);

// function to write word to memory or port
//typedef void (*pEmu16Write16)(u16 addr, u16 data);

// function to write dword to memory or port
//typedef void (*pEmu16Write32)(u16 addr, u32 data);

#if USE_EMU_I4040		// use I4004/I4040 CPU emulator
#include "emu_i4040.h"		// I4004/I4040 CPU
#endif

#if USE_EMU_I8008		// use I8008 CPU emulator
#include "emu_i8008.h"		// I8008 CPU
#endif

#if USE_EMU_I8080		// use I8080 CPU emulator
#include "emu_i8080.h"		// I8080 CPU
#endif

#if USE_EMU_I8085		// use I8085 CPU emulator
#include "emu_i8085.h"		// I8085 CPU
#endif

#if USE_EMU_Z80			// use Z80 CPU emulator
#include "emu_z80.h"		// Z80 CPU
#endif

#ifdef __cplusplus
}
#endif

#endif // _EMU_H

#endif // USE_EMU
