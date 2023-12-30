
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

#include "../../global.h"	// globals

#if USE_EMU			// use emulators

#include "../../_sdk/inc/sdk_pwm.h"
#include "../../_sdk/inc/sdk_multicore.h"

#include "emu.h"

#if EMU_DEBUG_SYNC	// 1 = debug measure time synchronization
volatile Bool EmuDebClockRes;	// request to reset measure
u32 EmuDebClockOld;		// old clock counter
volatile u32 EmuDebClockTotal;	// sum of total clock intervals
volatile u32 EmuDebClockUsed;	// sum of used clock intervals
volatile s32 EmuDebClockMax;	// max used clock interval
volatile u32 EmuDebClockMaxTot;	// total clock interval on max
#endif // EMU_DEBUG_SYNC

#if USE_EMU_I4040		// use I4004/I4040 CPU emulator
#include "emu_i4040.c"		// I4004/I4040 CPU
#endif

#if USE_EMU_I8008		// use I8008 CPU emulator
#include "emu_i8008.c"		// I8008 CPU
#endif

#if USE_EMU_I8080		// use I8080 CPU emulator
#include "emu_i8080.c"		// I8080 CPU
#endif

#if USE_EMU_I8085		// use I8085 CPU emulator
#include "emu_i8085.c"		// I8085 CPU
#endif

#if USE_EMU_Z80			// use Z80 CPU emulator
#include "emu_z80.c"		// Z80 CPU
#endif

// initialize time synchronization (initialize PWM counter; returns real frequency in Hz)
//  pwm ... index of (unused) PWM slice (0..7)
//  freq ... required frequency in Hz (for 125 MHz system clock supported range: 488 kHz to 125 MHz)
u32 EmuSyncInit(sEmuSync* s, int pwm, u32 freq)
{
	// reset PWM to default state
	PWM_Reset(pwm);

	// set PWM frequency
	PWM_Clock(pwm, freq);

	// set period to 16 bits
	PWM_Top(pwm, 0xffff);

	// enable PWM
	PWM_Enable(pwm);

	// save pointer to PWM counter
	s->timer = PWM_CTR(pwm);

	// start synchronization
	EmuSyncStart(s);

	// return real frequency
	return PWM_GetClock(pwm);
}

// terminate time synchronization (stop PWM counter)
//  pwm ... index of used PWM slice (0..7)
void EmuSyncTerm(int pwm)
{
	// reset PWM to default state
	PWM_Reset(pwm);
}

#endif // USE_EMU
